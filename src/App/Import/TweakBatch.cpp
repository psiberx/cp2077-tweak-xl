#include "TweakBatch.hpp"

App::TweakBatch::TweakBatch(TweakDB::Manager& aManager)
    : m_manager(aManager)
{
}

bool App::TweakBatch::SetFlat(RED4ext::TweakDBID aFlatId, const RED4ext::CBaseRTTIType* aType,
                              const Core::SharedPtr<void>& aValue)
{
    if (!aFlatId.IsValid() || !aType || !aValue)
        return false;

    auto& entry = m_pendingFlats[aFlatId];
    entry.type = aType;
    entry.value = aValue;

    return true;
}

bool App::TweakBatch::MakeRecord(RED4ext::TweakDBID aRecordId, const RED4ext::CClass* aType,
                                 RED4ext::TweakDBID aSourceId)
{
    if (!aRecordId.IsValid() || !aType)
        return false;

    auto& entry = m_pendingRecords[aRecordId];

    if (!entry.type)
        m_orderedRecords.push_back(aRecordId);

    entry.type = aType;
    entry.sourceId = aSourceId;

    return true;
}

bool App::TweakBatch::AssociateRecord(RED4ext::TweakDBID aRecordId, RED4ext::CName aPropName,
                                      RED4ext::TweakDBID aFlatId)
{
    m_associationMap[aFlatId] = aRecordId;

    return true;
}

bool App::TweakBatch::AppendElement(RED4ext::TweakDBID aFlatId, const RED4ext::CBaseRTTIType* aType,
                                    const Core::SharedPtr<void>& aValue, bool aUnique)
{
    if (!aFlatId.IsValid() || !aType || !aValue)
        return false;

    auto& entry = m_pendingAlterings[aFlatId];
    entry.items.emplace_back(AlteringMethod::Append, aType, aValue, aUnique);

    return true;
}

bool App::TweakBatch::AppendFrom(RED4ext::TweakDBID aFlatId, RED4ext::TweakDBID aSourceId)
{
    if (!aFlatId.IsValid() || !aSourceId.IsValid())
        return false;

    auto& entry = m_pendingAlterings[aFlatId];
    entry.merges.emplace_back(AlteringMethod::Append, aSourceId);

    return true;
}

bool App::TweakBatch::RegisterName(RED4ext::TweakDBID aId, const std::string& aName)
{
    m_pendingNames[aId] = aName;

    return true;
}

bool App::TweakBatch::IsEmpty()
{
    return m_pendingFlats.empty() && m_pendingRecords.empty() && m_pendingAlterings.empty() && m_pendingNames.empty();
}

const App::TweakBatch::FlatEntry* App::TweakBatch::GetFlat(RED4ext::TweakDBID aFlatId)
{
    const auto it = m_pendingFlats.find(aFlatId);

    if (it == m_pendingFlats.end())
        return nullptr;

    return &it->second;
}

const App::TweakBatch::RecordEntry* App::TweakBatch::GetRecord(RED4ext::TweakDBID aRecordId)
{
    const auto it = m_pendingRecords.find(aRecordId);

    if (it == m_pendingRecords.end())
        return nullptr;

    return &it->second;
}

void App::TweakBatch::Dispatch()
{
    for (const auto& item : m_pendingNames)
    {
        m_manager.RegisterName(item.first, item.second);
    }

    Core::Set<RED4ext::TweakDBID> foreignKeys;

    for (const auto& item : m_pendingFlats)
    {
        const auto& flatId = item.first;
        const auto& flatType = item.second.type;
        const auto& flatValue = item.second.value.get();

        const auto success = m_manager.SetFlat(flatId, flatType, flatValue);

        if (!success)
        {
            LogError("Cannot set flat [{}].", AsString(flatId));
            continue;
        }

        if (TweakDB::RTDB::IsForeignKey(flatType))
        {
            const auto foreignKey = reinterpret_cast<RED4ext::TweakDBID*>(flatValue);
            foreignKeys.insert(*foreignKey);
        }
        else if (TweakDB::RTDB::IsForeignKeyArray(flatType))
        {
            const auto foreignKeyList = reinterpret_cast<RED4ext::DynArray<RED4ext::TweakDBID>*>(flatValue);
            for (const auto& foreignKey : *foreignKeyList)
                foreignKeys.insert(foreignKey);
        }
    }

    for (const auto& recordId : m_orderedRecords)
    {
        const auto& recordEntry = m_pendingRecords[recordId];

        if (m_manager.IsRecordExists(recordId))
        {
            const auto success = m_manager.UpdateRecord(recordId);

            if (!success)
            {
                LogError("Cannot update record [{}].", AsString(recordId));
                continue;
            }
        }
        else if (recordEntry.sourceId.IsValid())
        {
            const auto success = m_manager.CloneRecord(recordId, recordEntry.sourceId);

            if (!success)
            {
                LogError("Cannot clone record [{}] from [{}].", AsString(recordId), AsString(recordEntry.sourceId));
                continue;
            }
        }
        else
        {
            const auto success = m_manager.CreateRecord(recordId, recordEntry.type);

            if (!success)
            {
                LogError("Cannot create record [{}] of type [{}].", AsString(recordId), AsString(recordEntry.type));
                continue;
            }
        }
    }

    for (const auto& altering : m_pendingAlterings)
    {
        const auto& flatId = altering.first;
        const auto& flatData = m_manager.GetFlat(flatId);

        if (!flatData.value)
        {
            LogError("Cannot append to [{}], the flat doesn't exist.", AsString(flatId));
            continue;
        }

        if (flatData.type->GetType() != RED4ext::ERTTIType::Array)
        {
            LogError("Cannot append to [{}] as it's not an array.", AsString(flatId));
            continue;
        }

        const auto isForeignKey = TweakDB::RTDB::IsForeignKeyArray(flatData.type);

        auto* targetType = reinterpret_cast<RED4ext::CRTTIArrayType*>(flatData.type);

        // Flat data returned by managed is a direct pointer to the TweakDB data buffer,
        // so we have to make a copy of that array for all modifications.
        auto targetArray = TweakDB::RTDB::MakeDefaultValue(targetType);
        targetType->Assign(targetArray.get(), flatData.value);

        auto newIndex = targetType->GetLength(targetArray.get());

        for (const auto& item : altering.second.items)
        {
            const auto newItem = item.value.get();

            if (item.unique && InArray(targetType, targetArray.get(), newItem))
                continue;

            targetType->InsertAt(targetArray.get(), newIndex);
            targetType->innerType->Assign(targetType->GetElement(targetArray.get(), newIndex), newItem);

            ++newIndex;

            if (isForeignKey)
            {
                const auto foreignKey = reinterpret_cast<RED4ext::TweakDBID*>(newItem);
                foreignKeys.insert(*foreignKey);
            }
        }

        for (const auto& merge : altering.second.merges)
        {
            const auto sourceData = m_manager.GetFlat(merge.sourceId);

            if (!sourceData.value || sourceData.type != targetType)
            {
                LogError("Cannot append from [{}] to [{}] as it's not an array.",
                         AsString(merge.sourceId), AsString(flatId));
                continue;
            }

            auto* sourceArray = reinterpret_cast<RED4ext::DynArray<void>*>(sourceData.value);
            const auto sourceLength = targetType->GetLength(sourceArray);

            for (uint32_t sourceIndex = 0; sourceIndex < sourceLength; ++sourceIndex)
            {
                auto sourceItem = targetType->GetElement(sourceArray, sourceIndex);

                if (InArray(targetType, targetArray.get(), sourceItem))
                    continue;

                targetType->InsertAt(targetArray.get(), newIndex);
                targetType->innerType->Assign(targetType->GetElement(targetArray.get(), newIndex), sourceItem);

                ++newIndex;
            }
        }

        const auto success = m_manager.SetFlat(flatId, targetType, targetArray.get());

        if (!success)
        {
            LogError("Cannot update flat [{}].", AsString(flatId));
            continue;
        }

        const auto assocIt = m_associationMap.find(flatId);

        if (assocIt != m_associationMap.end())
            m_manager.UpdateRecord(assocIt.value());
    }

    for (const auto& foreignKey : foreignKeys)
    {
        if (!m_manager.IsRecordExists(foreignKey) && !m_manager.IsFlatExists(foreignKey))
            LogWarning("Foreign key [{}] refers to a non-existent record / flat.", AsString(foreignKey));
    }

    m_pendingFlats.clear();
    m_pendingRecords.clear();
    m_orderedRecords.clear();
    m_pendingNames.clear();
    m_pendingAlterings.clear();
}

bool App::TweakBatch::InArray(RED4ext::CRTTIArrayType* aArrayType, RED4ext::ScriptInstance aArray,
                              RED4ext::ScriptInstance aValue)
{
    const auto length = aArrayType->GetLength(aArray);

    for (uint32_t i = 0; i < length; ++i)
    {
        if (aArrayType->innerType->IsEqual(aArrayType->GetElement(aArray, i), aValue))
            return true;
    }

    return false;
}

std::string App::TweakBatch::AsString(const RED4ext::CBaseRTTIType* aType)
{
    return aType->GetName().ToString();
}

std::string App::TweakBatch::AsString(RED4ext::TweakDBID aId)
{
    const auto nameIt = m_pendingNames.find(aId);

    if (nameIt != m_pendingNames.end())
        return nameIt->second;

    return fmt::format("<TDBID:{:08X}:{:02X}>", aId.name.hash, aId.name.length);
}
