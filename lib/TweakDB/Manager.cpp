#include "Manager.hpp"

namespace
{
constexpr bool OptimizeBatchFlatInsertions = true;
}

TweakDB::Manager::Manager()
    : Manager(RED4ext::TweakDB::Get())
{
}

TweakDB::Manager::Manager(RED4ext::TweakDB* aTweakDb)
    : m_tweakDb(aTweakDb)
    , m_flatPool(aTweakDb)
    , m_reflection(aTweakDb)
{
}

RED4ext::CStackType TweakDB::Manager::GetFlat(RED4ext::TweakDBID aFlatId)
{
    if (!aFlatId.IsValid())
        return {};

    std::shared_lock<RED4ext::SharedMutex> flatLockR(m_tweakDb->mutex00);

    auto* existing = m_tweakDb->flats.Find(aFlatId);

    if (existing == m_tweakDb->flats.End())
        return {};

    return m_flatPool.GetData(existing->ToTDBOffset());
}

RED4ext::Handle<RED4ext::gamedataTweakDBRecord> TweakDB::Manager::GetRecord(RED4ext::TweakDBID aRecordId)
{
    RED4ext::Handle<RED4ext::gamedataTweakDBRecord> record;

    if (!aRecordId.IsValid())
        return std::move(record);

    std::shared_lock<RED4ext::SharedMutex> recordLockR(m_tweakDb->mutex01);

    auto* existing = m_tweakDb->recordsByID.Get(aRecordId);

    recordLockR.unlock();

    if (!existing)
        return std::move(record);

    record = *reinterpret_cast<RED4ext::Handle<RED4ext::gamedataTweakDBRecord>*>(existing);

    return std::move(record);
}

bool TweakDB::Manager::IsFlatExists(RED4ext::TweakDBID aFlatId)
{
    if (!aFlatId.IsValid())
        return false;

    std::shared_lock<RED4ext::SharedMutex> flatLockR(m_tweakDb->mutex00);

    return m_tweakDb->flats.Find(aFlatId) != m_tweakDb->flats.End();
}

bool TweakDB::Manager::IsRecordExists(RED4ext::TweakDBID aRecordId)
{
    if (!aRecordId.IsValid())
        return false;

    std::shared_lock<RED4ext::SharedMutex> recordLockR(m_tweakDb->mutex01);

    return m_tweakDb->recordsByID.Get(aRecordId) != nullptr;
}

bool TweakDB::Manager::SetFlat(RED4ext::TweakDBID aFlatId, const RED4ext::CBaseRTTIType* aType,
                               RED4ext::ScriptInstance aValue)
{
    if (!aValue)
        return false;

    if (!aFlatId.IsValid())
        return false;

    if (!RTDB::IsFlatType(aType))
        return false;

    std::shared_lock<RED4ext::SharedMutex> flatLockR(m_tweakDb->mutex00);

    auto* existing = m_tweakDb->flats.Find(aFlatId);

    if (existing == m_tweakDb->flats.End())
    {
        flatLockR.unlock();

        auto offset = m_flatPool.AllocateValue(aType, aValue);

        if (offset == -1)
            return false;

        auto entry = RED4ext::TweakDBID(aFlatId);
        entry.SetTDBOffset(offset);

        std::lock_guard<RED4ext::SharedMutex> flatLockRW(m_tweakDb->mutex00);

        auto result = m_tweakDb->flats.Insert(entry);

        if (!result.second)
            return false;
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

            if (offset == -1)
                return false;

            existing->SetTDBOffset(offset);
        }
    }

    return true;
}

bool TweakDB::Manager::SetFlat(RED4ext::TweakDBID aFlatId, RED4ext::CStackType aData)
{
    return SetFlat(aFlatId, aData.type, aData.value);
}

bool TweakDB::Manager::CreateRecord(RED4ext::TweakDBID aRecordId, const RED4ext::CClass* aType)
{
    if (!RTDB::IsRecordType(aType))
        return false;

    std::shared_lock<RED4ext::SharedMutex> recordLockR(m_tweakDb->mutex01);

    auto* record = m_tweakDb->recordsByID.Get(aRecordId);

    recordLockR.unlock();

    // Abort if record already exists
    if (record)
        return false;

    const auto recordInfo = m_reflection.GetRecordInfo(aType);

    std::unique_lock<RED4ext::SharedMutex> flatLockRW(m_tweakDb->mutex00);

    if constexpr (OptimizeBatchFlatInsertions)
    {
        decltype(m_tweakDb->flats) propFlats;
        propFlats.Reserve(recordInfo->props.size());

        for (const auto& propIt : recordInfo->props)
        {
            const auto& propInfo = propIt.second;

            auto propFlat = RED4ext::TweakDBID(aRecordId, propInfo->appendix);
            propFlat.SetTDBOffset(m_flatPool.AllocateDefault(propInfo->type));

            propFlats.Emplace(propFlat);
        }

        m_tweakDb->flats.Insert(propFlats);
    }
    else
    {
        for (const auto& propIt : recordInfo->props)
        {
            const auto& propInfo = propIt.second;
            auto propId = RED4ext::TweakDBID(aRecordId, propInfo->appendix);

            if (m_tweakDb->flats.Find(propId) == m_tweakDb->flats.End())
            {
                propId.SetTDBOffset(m_flatPool.AllocateDefault(propInfo->type));
                m_tweakDb->flats.Insert(propId);
            }
        }
    }

    flatLockRW.unlock();
    recordLockR.lock();

    auto* first = m_tweakDb->recordsByType.Get(const_cast<RED4ext::CClass*>(aType))->Begin();

    recordLockR.unlock();

    CreateRecordInternal(m_tweakDb, GetRecordTypeHash(first), aRecordId);

    return true;
}

bool TweakDB::Manager::CloneRecord(RED4ext::TweakDBID aRecordId, RED4ext::TweakDBID aSourceId)
{
    if (!aRecordId.IsValid())
        return false;

    std::shared_lock<RED4ext::SharedMutex> recordLockR(m_tweakDb->mutex01);

    auto* record = m_tweakDb->recordsByID.Get(aRecordId);
    auto* source = m_tweakDb->recordsByID.Get(aSourceId);

    recordLockR.unlock();

    // Abort if record already exists or source doesn't exist
    if (record || !source)
        return false;

    const auto recordInfo = m_reflection.GetRecordInfo(source->GetPtr()->GetType());

    std::unique_lock<RED4ext::SharedMutex> flatLockRW(m_tweakDb->mutex00);

    if constexpr (OptimizeBatchFlatInsertions)
    {
        decltype(m_tweakDb->flats) propFlats;
        propFlats.Reserve(recordInfo->props.size());

        for (const auto& propIt : recordInfo->props)
        {
            const auto& propInfo = propIt.second;

            const auto sourcePropId = RED4ext::TweakDBID(aSourceId, propInfo->appendix);
            const auto* sourceFlat = m_tweakDb->flats.Find(sourcePropId);

            assert(sourceFlat->IsValid());

            auto propFlat = RED4ext::TweakDBID(aRecordId, propInfo->appendix);
            propFlat.SetTDBOffset(sourceFlat->ToTDBOffset());

            propFlats.Emplace(propFlat);
        }

        m_tweakDb->flats.Insert(propFlats);
    }
    else
    {
        for (const auto& propIt : recordInfo->props)
        {
            const auto& propInfo = propIt.second;
            auto propId = RED4ext::TweakDBID(aRecordId, propInfo->appendix);

            if (m_tweakDb->flats.Find(propId) == m_tweakDb->flats.End())
            {
                const auto sourcePropId = RED4ext::TweakDBID(aSourceId, propInfo->appendix);
                const auto* sourceFlat = m_tweakDb->flats.Find(sourcePropId);

                assert(sourceFlat->IsValid());

                propId.SetTDBOffset(sourceFlat->ToTDBOffset());
                m_tweakDb->flats.Insert(propId);
            }
        }
    }

    flatLockRW.unlock();

    CreateRecordInternal(m_tweakDb, GetRecordTypeHash(source), aRecordId);

    return true;
}

bool TweakDB::Manager::UpdateRecord(RED4ext::TweakDBID aRecordId)
{
    if (!aRecordId.IsValid())
        return false;

    std::shared_lock<RED4ext::SharedMutex> recordLockR(m_tweakDb->mutex01);

    auto* record = m_tweakDb->recordsByID.Get(aRecordId);

    recordLockR.unlock();

    // Abort if record doesn't exist
    if (!record)
        return false;

    auto* handle = reinterpret_cast<RED4ext::Handle<RED4ext::gamedataTweakDBRecord>*>(record);
    auto success = m_tweakDb->UpdateRecord(handle->GetPtr());

    // TODO: Update offsets directly?

    if (!success)
        return false;

    return true;
}

void TweakDB::Manager::RegisterName(RED4ext::TweakDBID aId, const std::string& aName)
{
    RED4ext::TweakDBID base;
    CreateTweakDBID(&base, &aId, aName.c_str());
}

uint32_t TweakDB::Manager::GetRecordTypeHash(RED4ext::Handle<RED4ext::IScriptable>* aRecord)
{
    return reinterpret_cast<RED4ext::Handle<RED4ext::gamedataTweakDBRecord>*>(aRecord)->GetPtr()->GetTweakBaseHash();
}

TweakDB::Reflection& TweakDB::Manager::GetReflection()
{
    return m_reflection;
}
