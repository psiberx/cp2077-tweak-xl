#include "Manager.hpp"
#include "Red/TweakDB/Raws.hpp"

namespace
{
constexpr auto OptimizedFlatChunkSize = 16000;
}

Red::TweakDBManager::TweakDBManager()
    : TweakDBManager(Red::TweakDB::Get())
{
}

Red::TweakDBManager::TweakDBManager(Red::TweakDB* aTweakDb)
    : m_tweakDb(aTweakDb)
    , m_buffer(Core::MakeShared<Red::TweakDBBuffer>(m_tweakDb))
    , m_reflection(Core::MakeShared<Red::TweakDBReflection>(m_tweakDb))
{
}

Red::TweakDBManager::TweakDBManager(Core::SharedPtr<Red::TweakDBReflection> aReflection)
    : m_tweakDb(aReflection->GetTweakDB())
    , m_buffer(Core::MakeShared<Red::TweakDBBuffer>(m_tweakDb))
    , m_reflection(std::move(aReflection))
{
}

Red::Value<> Red::TweakDBManager::GetFlat(Red::TweakDBID aFlatId)
{
    int32_t offset;

    {
        std::shared_lock flatLockR(m_tweakDb->mutex00);
        auto* flat = m_tweakDb->flats.Find(aFlatId);

        if (flat == m_tweakDb->flats.End())
            return {};

        offset = flat->ToTDBOffset();
    }

    return m_buffer->GetValue(offset);
}

Red::Value<> Red::TweakDBManager::GetDefault(const Red::CBaseRTTIType* aType)
{
    if (!m_reflection->IsFlatType(aType))
        return {};

    return m_buffer->GetValue(m_buffer->AllocateDefault(aType));
}

Red::Handle<Red::TweakDBRecord> Red::TweakDBManager::GetRecord(Red::TweakDBID aRecordId)
{
    std::shared_lock recordLockR(m_tweakDb->mutex01);
    const auto* record = m_tweakDb->recordsByID.Get(aRecordId);

    if (record == nullptr)
        return {};

    return *reinterpret_cast<const Red::Handle<Red::TweakDBRecord>*>(record);
}

const Red::CClass* Red::TweakDBManager::GetRecordType(Red::TweakDBID aRecordId)
{
    std::shared_lock recordLockR(m_tweakDb->mutex01);
    const auto* record = m_tweakDb->recordsByID.Get(aRecordId);
    return record ? record->GetPtr()->GetType() : nullptr;
}

bool Red::TweakDBManager::IsFlatExists(Red::TweakDBID aFlatId)
{
    std::shared_lock flatLockR(m_tweakDb->mutex00);
    return m_tweakDb->flats.Find(aFlatId) != m_tweakDb->flats.End();
}

bool Red::TweakDBManager::IsRecordExists(Red::TweakDBID aRecordId)
{
    std::shared_lock recordLockR(m_tweakDb->mutex01);
    return m_tweakDb->recordsByID.Get(aRecordId) != nullptr;
}

bool Red::TweakDBManager::SetFlat(Red::TweakDBID aFlatId, const Red::CBaseRTTIType* aType, Red::Instance aInstance)
{
    if (!aFlatId.IsValid() || !aInstance || !m_reflection->IsFlatType(aType))
        return false;

    return AssignFlat(m_tweakDb->flats, aFlatId, aType, aInstance, m_tweakDb->mutex00);
}

bool Red::TweakDBManager::SetFlat(Red::TweakDBID aFlatId, const Red::Value<>& aData)
{
    return SetFlat(aFlatId, aData.type, aData.instance);
}

bool Red::TweakDBManager::CreateRecord(Red::TweakDBID aRecordId, const Red::CClass* aType)
{
    if (!aRecordId.IsValid() || IsRecordExists(aRecordId))
        return false;

    const auto recordInfo = m_reflection->GetRecordInfo(aType);

    if (!recordInfo)
        return false;

    Red::SortedUniqueArray<Red::TweakDBID> propFlats;
    propFlats.Reserve(recordInfo->props.size());
    InheritFlats(propFlats, aRecordId, recordInfo);

    {
        std::unique_lock flatLockRW(m_tweakDb->mutex00);
        m_tweakDb->flats.Insert(propFlats);
    }

    Raw::CreateRecord(m_tweakDb, recordInfo->typeHash, aRecordId);

    return true;
}

bool Red::TweakDBManager::CloneRecord(Red::TweakDBID aRecordId, Red::TweakDBID aSourceId)
{
    if (!aRecordId.IsValid() || !aSourceId.IsValid())
        return false;

    if (IsRecordExists(aRecordId) || !IsRecordExists(aSourceId))
        return false;

    const auto recordType = GetRecordType(aSourceId);
    const auto recordInfo = m_reflection->GetRecordInfo(recordType);

    if (!recordInfo)
        return false;

    Red::SortedUniqueArray<Red::TweakDBID> propFlats;
    propFlats.Reserve(recordInfo->props.size());
    InheritFlats(propFlats, aRecordId, recordInfo, aSourceId);

    {
        std::unique_lock flatLockRW(m_tweakDb->mutex00);
        m_tweakDb->flats.Insert(propFlats);
    }

    Raw::CreateRecord(m_tweakDb, recordInfo->typeHash, aRecordId);

    return true;
}

bool Red::TweakDBManager::InheritProps(Red::TweakDBID aRecordId, Red::TweakDBID aSourceId)
{
    if (!aRecordId.IsValid() || !aSourceId.IsValid())
        return false;

    if (!IsRecordExists(aRecordId) || !IsRecordExists(aSourceId))
        return false;

    const auto recordType = GetRecordType(aRecordId);
    const auto sourceType = GetRecordType(aSourceId);

    if (recordType != sourceType)
        return false;

    const auto recordInfo = m_reflection->GetRecordInfo(recordType);

    if (!recordInfo)
        return false;

    Red::SortedUniqueArray<Red::TweakDBID> propFlats;
    propFlats.Reserve(recordInfo->props.size());
    InheritFlats(propFlats, aRecordId, recordInfo, aSourceId);

    {
        std::unique_lock flatLockRW(m_tweakDb->mutex00);
        m_tweakDb->flats.Insert(propFlats);
    }

    return true;
}

bool Red::TweakDBManager::UpdateRecord(Red::TweakDBID aRecordId)
{
    if (!aRecordId.IsValid())
        return false;

    const auto record = GetRecord(aRecordId);

    if (!record)
        return false;

    std::unique_lock recordLockRW(m_tweakDb->mutex01);
    return m_tweakDb->UpdateRecord(record);
}

void Red::TweakDBManager::RegisterEnum(Red::TweakDBID aRecordId)
{
    m_knownEnums.insert(aRecordId);
}

void Red::TweakDBManager::RegisterName(const std::string& aName, const Red::CClass* aType)
{
    RegisterName(aName.data(), aName);
}

void Red::TweakDBManager::RegisterName(Red::TweakDBID aId, const std::string& aName, const Red::CClass* aType)
{
    CreateBaseName(aId, aName);
    CreateExtraNames(aId, aName, aType);
}

Red::TweakDBManager::BatchPtr Red::TweakDBManager::StartBatch()
{
    return Core::MakeShared<Batch>();
}

const Core::Set<Red::TweakDBID>& Red::TweakDBManager::GetFlats(const Red::TweakDBManager::BatchPtr& aBatch)
{
    return aBatch->flats;
}

Red::Value<> Red::TweakDBManager::GetFlat(const Red::TweakDBManager::BatchPtr& aBatch, Red::TweakDBID aFlatId)
{
    std::shared_lock flatLockR(aBatch->mutex);
    const auto& flat = aBatch->flats.find(aFlatId);

    if (flat == aBatch->flats.end())
        return {};

    return m_buffer->GetValue(flat->ToTDBOffset());
}

const Red::CClass* Red::TweakDBManager::GetRecordType(const Red::TweakDBManager::BatchPtr& aBatch, Red::TweakDBID aRecordId)
{
    auto recordType = GetRecordType(aRecordId);

    if (!recordType)
    {
        std::shared_lock batchLockR(aBatch->mutex);
        const auto it = aBatch->records.find(aRecordId);

        if (it != aBatch->records.end())
            recordType = it.value()->type;
    }

    return recordType;
}

bool Red::TweakDBManager::IsFlatExists(const Red::TweakDBManager::BatchPtr& aBatch, Red::TweakDBID aFlatId)
{
    if (IsFlatExists(aFlatId))
        return true;

    std::shared_lock batchLockR(aBatch->mutex);
    return aBatch->flats.contains(aFlatId);
}

bool Red::TweakDBManager::IsRecordExists(const Red::TweakDBManager::BatchPtr& aBatch, Red::TweakDBID aRecordId)
{
    if (IsRecordExists(aRecordId))
        return true;

    std::shared_lock batchLockR(aBatch->mutex);
    return aBatch->records.contains(aRecordId);
}

bool Red::TweakDBManager::SetFlat(const Red::TweakDBManager::BatchPtr& aBatch, Red::TweakDBID aFlatId,
                                  const Red::CBaseRTTIType* aType, Red::Instance aInstance)
{
    if (!aFlatId.IsValid() || !aInstance || !m_reflection->IsFlatType(aType))
        return false;

    return AssignFlat(aBatch, aFlatId, {aType, aInstance});
}

bool Red::TweakDBManager::SetFlat(const Red::TweakDBManager::BatchPtr& aBatch, Red::TweakDBID aFlatId,
                                  const Red::Value<>& aValue)
{
    if (!aFlatId.IsValid() || !aValue.instance || !m_reflection->IsFlatType(aValue.type))
        return false;

    return AssignFlat(aBatch, aFlatId, aValue);
}

bool Red::TweakDBManager::CreateRecord(const Red::TweakDBManager::BatchPtr& aBatch, Red::TweakDBID aRecordId,
                                       const Red::CClass* aType)
{
    if (!aRecordId.IsValid() || !aType || IsRecordExists(aBatch, aRecordId))
        return false;

    const auto recordInfo = m_reflection->GetRecordInfo(aType);

    if (!recordInfo)
        return false;

    std::unique_lock batchLockRW(aBatch->mutex);
    InheritFlats(aBatch, aRecordId, recordInfo);
    aBatch->records.emplace(aRecordId, recordInfo);

    return true;
}

bool Red::TweakDBManager::CloneRecord(const Red::TweakDBManager::BatchPtr& aBatch, Red::TweakDBID aRecordId,
                                      Red::TweakDBID aSourceId)
{
    if (!aRecordId.IsValid() || !aSourceId.IsValid())
        return false;

    if (IsRecordExists(aBatch, aRecordId) || !IsRecordExists(aBatch, aSourceId))
        return false;

    auto recordType = GetRecordType(aBatch, aSourceId);
    auto recordInfo = m_reflection->GetRecordInfo(recordType);

    if (!recordInfo)
        return false;

    std::unique_lock batchLockRW(aBatch->mutex);
    InheritFlats(aBatch, aRecordId, recordInfo, aSourceId);
    aBatch->records.emplace(aRecordId, recordInfo);

    return true;
}

bool Red::TweakDBManager::InheritProps(const Red::TweakDBManager::BatchPtr& aBatch, Red::TweakDBID aRecordId,
                                       Red::TweakDBID aSourceId)
{
    if (!aRecordId.IsValid() || !aSourceId.IsValid())
        return false;

    if (!IsRecordExists(aBatch, aRecordId) || !IsRecordExists(aBatch, aSourceId))
        return false;

    const auto recordType = GetRecordType(aBatch, aRecordId);
    const auto sourceType = GetRecordType(aBatch, aSourceId);

    if (recordType != sourceType)
        return false;

    auto recordInfo = m_reflection->GetRecordInfo(recordType);

    if (!recordInfo)
        return false;

    std::unique_lock batchLockRW(aBatch->mutex);
    InheritFlats(aBatch, aRecordId, recordInfo, aSourceId);

    return true;
}

bool Red::TweakDBManager::UpdateRecord(const Red::TweakDBManager::BatchPtr& aBatch, Red::TweakDBID aRecordId)
{
    if (!aRecordId.IsValid() || !IsRecordExists(aRecordId))
        return false;

    std::unique_lock batchLockRW(aBatch->mutex);

    if (aBatch->records.contains(aRecordId))
        return false;

    aBatch->records.emplace(aRecordId, nullptr);

    return true;
}

void Red::TweakDBManager::RegisterEnum(const Red::TweakDBManager::BatchPtr& aBatch, Red::TweakDBID aRecordId)
{
    RegisterEnum(aRecordId);
}

void Red::TweakDBManager::RegisterName(const Red::TweakDBManager::BatchPtr& aBatch, Red::TweakDBID aId,
                                       const std::string& aName)
{
    std::unique_lock batchLockRW(aBatch->mutex);
    aBatch->names.emplace(aId, aName);
}

void Red::TweakDBManager::CommitBatch(const BatchPtr& aBatch)
{
    std::unique_lock batchLockRW(aBatch->mutex);

    for (const auto& [id, name] : aBatch->names)
    {
        CreateBaseName(id, name);
    }

    {
        Red::SortedUniqueArray<Red::TweakDBID> flatsChunk;

        for (const auto& flatId : aBatch->flats)
        {
            flatsChunk.InsertOrAssign(flatId);

            if (flatsChunk.size >= OptimizedFlatChunkSize)
            {
                std::unique_lock flatLockRW(m_tweakDb->mutex00);
                m_tweakDb->flats.InsertOrAssign(flatsChunk);
                flatsChunk.Clear();
            }
        }

        if (flatsChunk.size > 0)
        {
            std::unique_lock flatLockRW(m_tweakDb->mutex00);
            m_tweakDb->flats.InsertOrAssign(flatsChunk);
        }
    }

    for (const auto& [recordId, recordInfo] : aBatch->records)
    {
        const auto record = GetRecord(recordId);

        if (record)
        {
            std::unique_lock recordLockRW(m_tweakDb->mutex01);
            m_tweakDb->UpdateRecord(record);
        }
        else
        {
            Raw::CreateRecord(m_tweakDb, recordInfo->typeHash, recordId);
        }
    }

    for (const auto& [id, name] : aBatch->names)
    {
        CreateExtraNames(id, name);
    }

    aBatch->flats.clear();
    aBatch->records.clear();
    aBatch->names.clear();
}

void Red::TweakDBManager::Invalidate()
{
    m_buffer->Invalidate();
}

Red::TweakDB* Red::TweakDBManager::GetTweakDB()
{
    return m_tweakDb;
}

Core::SharedPtr<Red::TweakDBReflection>& Red::TweakDBManager::GetReflection()
{
    return m_reflection;
}

template<class SharedLockable>
bool Red::TweakDBManager::AssignFlat(Red::SortedUniqueArray<Red::TweakDBID>& aFlats, Red::TweakDBID aFlatId,
                                     const Red::CBaseRTTIType* aType, Red::Instance aInstance,
                                     SharedLockable& aMutex)
{
    int32_t offset = -1;

    {
        std::shared_lock flatLockR(aMutex);
        auto* flat = aFlats.Find(aFlatId);
        if (flat != aFlats.End())
        {
            offset = flat->ToTDBOffset();
        }
    }

    if (offset >= 0)
    {
        const auto value = m_buffer->GetValue(offset);

        if (value.type != aType)
            return false;

        if (value.type->IsEqual(value.instance, aInstance))
            return true;
    }

    offset = m_buffer->AllocateValue(aType, aInstance);

    if (offset < 0)
        return false;

    aFlatId.SetTDBOffset(offset);

    {
        std::unique_lock flatLockRW(aMutex);
        aFlats.InsertOrAssign(aFlatId);
    }

    return true;
}

void Red::TweakDBManager::InheritFlats(RED4ext::SortedUniqueArray<Red::TweakDBID>& aFlats, Red::TweakDBID aRecordId,
                                       const Red::TweakDBRecordInfo* aRecordInfo)
{
    for (const auto& [_, propInfo] : aRecordInfo->props)
    {
        if (!propInfo->dataOffset)
            continue;

        auto propFlat = Red::TweakDBID(aRecordId, propInfo->appendix);
        auto propDefault = propInfo->defaultValue;

        if (propDefault < 0)
        {
            propDefault = m_buffer->AllocateDefault(propInfo->type);
        }

        propFlat.SetTDBOffset(propDefault);
        aFlats.Emplace(propFlat);
    }
}

void Red::TweakDBManager::InheritFlats(RED4ext::SortedUniqueArray<Red::TweakDBID>& aFlats, Red::TweakDBID aRecordId,
                                       const Red::TweakDBRecordInfo* aRecordInfo, Red::TweakDBID aSourceId)
{
    std::shared_lock flatLockR(m_tweakDb->mutex00);

    for (const auto& [_, propInfo] : aRecordInfo->props)
    {
        const auto baseId = aSourceId + propInfo->appendix;
        const auto* baseFlat = aFlats.Find(baseId);

        if (baseFlat == aFlats.End())
        {
            baseFlat = m_tweakDb->flats.Find(baseId);
            if (baseFlat == m_tweakDb->flats.End())
                continue;
        }

        auto propFlat = aRecordId + propInfo->appendix;
        propFlat.SetTDBOffset(baseFlat->ToTDBOffset());

        aFlats.Emplace(propFlat);
    }
}

bool Red::TweakDBManager::AssignFlat(const Red::TweakDBManager::BatchPtr& aBatch, Red::TweakDBID aFlatId,
                                     const Red::Value<>& aValue)
{
    std::unique_lock batchLockRW(aBatch->mutex);

    const auto& flat = aBatch->flats.find(aFlatId);
    int32_t offset = -1;

    if (flat != aBatch->flats.end())
    {
        offset = flat->ToTDBOffset();

        const auto value = m_buffer->GetValue(offset);

        if (value.type != aValue.type)
            return false;

        if (value.type->IsEqual(value.instance, aValue.instance))
            return true;
    }

    offset = m_buffer->AllocateValue(aValue);

    if (offset < 0)
        return false;

    aFlatId.SetTDBOffset(offset);

    if (flat != aBatch->flats.end())
    {
        const_cast<Red::TweakDBID&>(*flat) = aFlatId;
    }
    else
    {
        aBatch->flats.insert(aFlatId);
    }

    return true;
}

void Red::TweakDBManager::InheritFlats(const Red::TweakDBManager::BatchPtr& aBatch, Red::TweakDBID aRecordId,
                                       const Red::TweakDBRecordInfo* aRecordInfo)
{
    for (const auto& [_, propInfo] : aRecordInfo->props)
    {
        if (!propInfo->dataOffset)
            continue;

        auto propFlat = Red::TweakDBID(aRecordId, propInfo->appendix);

        if (!aBatch->flats.contains(propFlat))
        {
            auto propDefault = propInfo->defaultValue;

            if (propDefault < 0)
            {
                propDefault = m_buffer->AllocateDefault(propInfo->type);
            }

            propFlat.SetTDBOffset(propDefault);

            aBatch->flats.insert(propFlat);
        }
    }
}

void Red::TweakDBManager::InheritFlats(const Red::TweakDBManager::BatchPtr& aBatch, Red::TweakDBID aRecordId,
                                       const Red::TweakDBRecordInfo* aRecordInfo, Red::TweakDBID aSourceId)
{
    std::shared_lock flatLockR(m_tweakDb->mutex00);

    for (const auto& [_, propInfo] : aRecordInfo->props)
    {
        const auto baseId = aSourceId + propInfo->appendix;
        const auto baseFlat = aBatch->flats.find(baseId);

        if (baseFlat != aBatch->flats.end())
        {
            auto propFlat = aRecordId + propInfo->appendix;
            propFlat.SetTDBOffset(baseFlat->ToTDBOffset());

            aBatch->flats.insert(propFlat);
        }
        else
        {
            auto commitedFlat = m_tweakDb->flats.Find(baseId);
            if (commitedFlat != m_tweakDb->flats.End())
            {
                auto propFlat = aRecordId + propInfo->appendix;
                propFlat.SetTDBOffset(commitedFlat->ToTDBOffset());

                aBatch->flats.insert(propFlat);
            }
        }
    }
}

void Red::TweakDBManager::CreateBaseName(Red::TweakDBID aId, const std::string& aName)
{
    Red::TweakDBID empty;
    Raw::CreateTweakDBID(&empty, &aId, aName.c_str());

    m_knownNames[aId] = aName;
}

void Red::TweakDBManager::CreateExtraNames(Red::TweakDBID aId, const std::string& aName, const Red::CClass* aType)
{
    const auto recordInfo = m_reflection->GetRecordInfo(aType ? aType : GetRecordType(aId));

    if (!recordInfo)
        return;

    for (const auto& [_, propInfo] : recordInfo->props)
    {
        const auto propId = aId + propInfo->appendix;
        const auto propName = aName + propInfo->appendix;

        if (propInfo->dataOffset)
        {
            Raw::CreateTweakDBID(&aId, &propId, propInfo->appendix.c_str());
        }
        else
        {
            Red::TweakDBID empty;
            Raw::CreateTweakDBID(&empty, &propId, propName.c_str());
        }

        m_knownNames[propId] = propName;
    }
}

const std::string& Red::TweakDBManager::GetName(Red::TweakDBID aId)
{
    auto it = m_knownNames.find(aId);

    if (it == m_knownNames.end())
    {
        auto debugName = m_reflection->ToString(aId);
        if (!debugName.empty())
        {
            it = m_knownNames.emplace(aId, debugName).first;
        }
        else
        {
            auto hashName = std::format("<TDBID:{:08X}:{:02X}>", aId.name.hash, aId.name.length);
            it = m_knownNames.emplace(aId, hashName).first;
        }
    }

    return it->second;
}

const Core::Set<Red::TweakDBID>& Red::TweakDBManager::GetEnums()
{
    return m_knownEnums;
}
