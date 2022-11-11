#include "Manager.hpp"
#include "Raws.hpp"

Red::TweakDB::Manager::Manager()
    : Manager(Instance::Get())
{
}

Red::TweakDB::Manager::Manager(Instance* aTweakDb)
    : m_tweakDb(aTweakDb)
    , m_flatPool(aTweakDb)
    , m_reflection(aTweakDb)
    , m_batchMode(false)
{
}

Red::CStackType Red::TweakDB::Manager::GetFlat(Red::TweakDBID aFlatId)
{
    if (!aFlatId.IsValid())
        return {};

    std::shared_lock flatLockR(m_tweakDb->mutex00);
    auto* flat = m_tweakDb->flats.Find(aFlatId);

    if (flat == m_tweakDb->flats.End())
        return {};

    return m_flatPool.GetData(flat->ToTDBOffset());
}

Red::Handle<Red::gamedataTweakDBRecord> Red::TweakDB::Manager::GetRecord(Red::TweakDBID aRecordId)
{
    if (!aRecordId.IsValid())
        return {};

    Red::Handle<Red::IScriptable>* record;

    {
        std::shared_lock recordLockR(m_tweakDb->mutex01);
        record = m_tweakDb->recordsByID.Get(aRecordId);
    }

    if (!record)
        return {};

    return *reinterpret_cast<Red::Handle<Red::gamedataTweakDBRecord>*>(record);
}

bool Red::TweakDB::Manager::IsFlatExists(Red::TweakDBID aFlatId)
{
    if (!aFlatId.IsValid())
        return false;

    std::shared_lock flatLockR(m_tweakDb->mutex00);

    return m_tweakDb->flats.Find(aFlatId) != m_tweakDb->flats.End();
}

bool Red::TweakDB::Manager::IsRecordExists(Red::TweakDBID aRecordId)
{
    if (!aRecordId.IsValid())
        return false;

    std::shared_lock recordLockR(m_tweakDb->mutex01);

    return m_tweakDb->recordsByID.Get(aRecordId) != nullptr;
}

bool Red::TweakDB::Manager::SetFlat(Red::TweakDBID aFlatId, const Red::CBaseRTTIType* aType,
                               Red::ScriptInstance aValue)
{
    if (!aValue)
        return false;

    if (!aFlatId.IsValid())
        return false;

    if (!IsFlatType(aType))
        return false;

    std::shared_lock flatLockR(m_tweakDb->mutex00);

    auto* existing = m_tweakDb->flats.Find(aFlatId);

    if (existing == m_tweakDb->flats.End())
    {
        flatLockR.unlock();

        auto offset = m_flatPool.AllocateValue(aType, aValue);

        if (offset == FlatPool::InvalidOffset)
            return false;

        aFlatId.SetTDBOffset(offset);

        if (m_batchMode)
        {
            m_batchFlats.InsertOrAssign(aFlatId);
        }
        else
        {
            std::unique_lock flatLockRW(m_tweakDb->mutex00);

            auto result = m_tweakDb->flats.Insert(aFlatId);

            if (!result.second)
                return false;
        }
    }
    else
    {
        const auto data = m_flatPool.GetData(existing->ToTDBOffset());

        flatLockR.unlock();

        if (data.type != aType)
            return false;

        if (!data.type->IsEqual(data.value, aValue))
        {
            auto offset = m_flatPool.AllocateValue(aType, aValue);

            if (offset == FlatPool::InvalidOffset)
                return false;

            if (m_batchMode)
            {
                aFlatId.SetTDBOffset(offset);
                m_batchFlats.InsertOrAssign(aFlatId);
            }
            else
            {
                existing->SetTDBOffset(offset);
            }
        }
    }

    return true;
}

bool Red::TweakDB::Manager::SetFlat(Red::TweakDBID aFlatId, Red::CStackType aData)
{
    return SetFlat(aFlatId, aData.type, aData.value);
}

bool Red::TweakDB::Manager::CreateRecord(Red::TweakDBID aRecordId, const Red::CClass* aType)
{
    if (!IsRecordType(aType))
        return false;

    if (m_batchMode && m_batchRecords.contains(aRecordId))
        return false;

    Red::Handle<Red::IScriptable>* record;

    {
        std::shared_lock recordLockR(m_tweakDb->mutex01);
        record = m_tweakDb->recordsByID.Get(aRecordId);
    }

    // Abort if record already exists
    if (record)
        return false;

    const auto recordInfo = m_reflection.GetRecordInfo(aType);

    decltype(m_tweakDb->flats) propFlats;
    propFlats.Reserve(recordInfo->props.size());

    {
        std::shared_lock flatLockR(m_tweakDb->mutex00);
        for (const auto& propIt : recordInfo->props)
        {
            const auto& propInfo = propIt.second;

            auto propFlat = Red::TweakDBID(aRecordId, propInfo->appendix);

            if (propInfo->defaultValue > 0)
                propFlat.SetTDBOffset(propInfo->defaultValue);
            else
                propFlat.SetTDBOffset(m_flatPool.AllocateDefault(propInfo->type));

            propFlats.Emplace(propFlat);
        }
    }

    if (m_batchMode)
    {
        m_batchFlats.Insert(propFlats);
    }
    else
    {
        std::unique_lock flatLockRW(m_tweakDb->mutex00);
        m_tweakDb->flats.Insert(propFlats);
    }

    // {
    //     std::shared_lock recordLockR(m_tweakDb->mutex01);
    //     record = m_tweakDb->recordsByType.Get(const_cast<Red::CClass*>(aType))->Begin();
    // }

    if (m_batchMode)
    {
        m_batchRecords.emplace(aRecordId, recordInfo);
    }
    else
    {
        Raw::CreateRecord(m_tweakDb, recordInfo->typeHash, aRecordId);
    }

    return true;
}

bool Red::TweakDB::Manager::CloneRecord(Red::TweakDBID aRecordId, Red::TweakDBID aSourceId)
{
    if (!aRecordId.IsValid())
        return false;

    if (m_batchMode && m_batchRecords.contains(aRecordId))
        return false;

    Red::Handle<Red::IScriptable>* record;
    Red::Handle<Red::IScriptable>* source;

    {
        std::shared_lock recordLockR(m_tweakDb->mutex01);
        record = m_tweakDb->recordsByID.Get(aRecordId);
        source = m_tweakDb->recordsByID.Get(aSourceId);
    }

    // Abort if record already exists or source doesn't exist
    if (record)
        return false;

    bool batch = false;
    const RecordTypeInfo* recordInfo;

    if (source)
    {
        recordInfo = m_reflection.GetRecordInfo(source->GetPtr()->GetType());
    }
    else
    {
        if (!m_batchMode || !m_batchRecords.contains(aSourceId))
            return false;

        recordInfo = m_batchRecords.at(aSourceId);
        batch = true;
    }

    decltype(m_tweakDb->flats) propFlats;
    propFlats.Reserve(recordInfo->props.size());

    {
        std::shared_lock flatLockR(m_tweakDb->mutex00);
        for (const auto& propIt : recordInfo->props)
        {
            const auto& propInfo = propIt.second;

            const auto sourcePropId = Red::TweakDBID(aSourceId, propInfo->appendix);
            const auto* sourceFlat = batch ? m_batchFlats.Find(sourcePropId) : m_tweakDb->flats.Find(sourcePropId);

            assert(sourceFlat->IsValid());

            auto propFlat = Red::TweakDBID(aRecordId, propInfo->appendix);
            propFlat.SetTDBOffset(sourceFlat->ToTDBOffset());

            propFlats.Emplace(propFlat);
        }
    }

    if (m_batchMode)
    {
        m_batchFlats.Insert(propFlats);
    }
    else
    {
        std::unique_lock flatLockRW(m_tweakDb->mutex00);
        m_tweakDb->flats.Insert(propFlats);
    }

    if (m_batchMode)
    {
        m_batchRecords.emplace(aRecordId, recordInfo);
    }
    else
    {
        Raw::CreateRecord(m_tweakDb, recordInfo->typeHash, aRecordId);
    }

    return true;
}

bool Red::TweakDB::Manager::UpdateRecord(Red::TweakDBID aRecordId)
{
    if (!aRecordId.IsValid())
        return false;

    if (m_batchMode && m_batchRecords.contains(aRecordId))
        return true;

    Red::Handle<Red::IScriptable>* record;

    {
        std::shared_lock recordLockR(m_tweakDb->mutex01);
        record = m_tweakDb->recordsByID.Get(aRecordId);
    }

    // Abort if record doesn't exist
    if (!record)
        return false;

    if (m_batchMode)
    {
        m_batchRecords.emplace(aRecordId, nullptr);
    }
    else
    {
        if (!m_tweakDb->UpdateRecord(record->GetPtr<Red::gamedataTweakDBRecord>()))
            return false;
    }

    return true;
}

void Red::TweakDB::Manager::RegisterName(Red::TweakDBID aId, const std::string& aName)
{
    Red::TweakDBID base;
    Raw::CreateTweakDBID(&base, &aId, aName.c_str());
}

void Red::TweakDB::Manager::StartBatch()
{
    m_batchMode = true;
}

void Red::TweakDB::Manager::CommitBatch()
{
    if (!m_batchMode)
        return;

    {
        std::unique_lock flatLockRW(m_tweakDb->mutex00);
        m_tweakDb->flats.InsertOrAssign(m_batchFlats);
    }

    for (const auto& [recordId, recordInfo] : m_batchRecords)
    {
        if (recordInfo)
        {
            Raw::CreateRecord(m_tweakDb, recordInfo->typeHash, recordId);
        }
    }

    {
        std::unique_lock recordLockRW(m_tweakDb->mutex01);
        for (const auto& [recordId, recordInfo] : m_batchRecords)
        {
            if (!recordInfo)
            {
                auto* record = m_tweakDb->recordsByID.Get(recordId);
                m_tweakDb->UpdateRecord(record->GetPtr<Red::gamedataTweakDBRecord>());
            }
        }
    }

    m_batchFlats.Clear();
    m_batchRecords.clear();
    m_batchMode = false;
}

Red::TweakDB::Reflection& Red::TweakDB::Manager::GetReflection()
{
    return m_reflection;
}
