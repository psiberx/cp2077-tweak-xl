#include "TweakChangeset.hpp"

bool App::TweakChangeset::SetFlat(Red::TweakDBID aFlatId, const Red::CBaseRTTIType* aType,
                                  const Core::SharedPtr<void>& aValue)
{
    if (!aFlatId.IsValid() || !aType || !aValue)
        return false;

    auto& entry = m_pendingFlats[aFlatId];
    entry.type = aType;
    entry.value = aValue;

    return true;
}

bool App::TweakChangeset::MakeRecord(Red::TweakDBID aRecordId, const Red::CClass* aType,
                                     Red::TweakDBID aSourceId)
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

bool App::TweakChangeset::UpdateRecord(Red::TweakDBID aRecordId)
{
    if (!aRecordId.IsValid())
        return false;

    if (!m_pendingRecords.contains(aRecordId))
    {
        m_pendingRecords.insert({aRecordId, {}});
        m_orderedRecords.push_back(aRecordId);
    }

    return true;
}

bool App::TweakChangeset::AppendElement(Red::TweakDBID aFlatId, const Red::CBaseRTTIType* aType,
                                        const Core::SharedPtr<void>& aValue, bool aUnique)
{
    if (!aFlatId.IsValid() || !aType || !aValue)
        return false;

    auto& entry = m_pendingMutations[aFlatId];
    entry.appendings.emplace_back(aType, aValue, aUnique);

    return true;
}

bool App::TweakChangeset::PrependElement(Red::TweakDBID aFlatId, const Red::CBaseRTTIType* aType,
                                         const Core::SharedPtr<void>& aValue, bool aUnique)
{
    if (!aFlatId.IsValid() || !aType || !aValue)
        return false;

    auto& entry = m_pendingMutations[aFlatId];
    entry.prependings.emplace_back(aType, aValue, aUnique);

    return true;
}

bool App::TweakChangeset::RemoveElement(Red::TweakDBID aFlatId, const Red::CBaseRTTIType* aType,
                                        const Core::SharedPtr<void>& aValue)
{
    if (!aFlatId.IsValid() || !aType || !aValue)
        return false;

    auto& entry = m_pendingMutations[aFlatId];
    entry.deletions.emplace_back(aType, aValue);

    return true;
}

bool App::TweakChangeset::AppendFrom(Red::TweakDBID aFlatId, Red::TweakDBID aSourceId)
{
    if (!aFlatId.IsValid() || !aSourceId.IsValid())
        return false;

    auto& entry = m_pendingMutations[aFlatId];
    entry.appendingMerges.emplace_back(aSourceId);

    return true;
}

bool App::TweakChangeset::PrependFrom(Red::TweakDBID aFlatId, Red::TweakDBID aSourceId)
{
    if (!aFlatId.IsValid() || !aSourceId.IsValid())
        return false;

    auto& entry = m_pendingMutations[aFlatId];
    entry.prependingMerges.emplace_back(aSourceId);

    return true;
}

bool App::TweakChangeset::InheritChanges(Red::TweakDBID aFlatId, Red::TweakDBID aBaseId)
{
    if (!aFlatId.IsValid() || !aBaseId.IsValid())
        return false;

    if (m_pendingFlats.contains(aFlatId))
        return false;

    if (!m_pendingMutations.contains(aBaseId))
        return false;

    auto& entry = m_pendingMutations[aFlatId];
    entry.baseId = aBaseId;

    return true;
}

bool App::TweakChangeset::RegisterName(Red::TweakDBID aId, const std::string& aName)
{
    m_pendingNames[aId] = aName;

    return true;
}

const App::TweakChangeset::FlatEntry* App::TweakChangeset::GetFlat(Red::TweakDBID aFlatId)
{
    const auto it = m_pendingFlats.find(aFlatId);

    if (it == m_pendingFlats.end())
        return nullptr;

    return &it->second;
}

const App::TweakChangeset::RecordEntry* App::TweakChangeset::GetRecord(Red::TweakDBID aRecordId)
{
    const auto it = m_pendingRecords.find(aRecordId);

    if (it == m_pendingRecords.end())
        return nullptr;

    return &it->second;
}

bool App::TweakChangeset::HasRecord(Red::TweakDBID aRecordId)
{
    return m_pendingRecords.find(aRecordId) != m_pendingRecords.end();
}

bool App::TweakChangeset::IsEmpty()
{
    return m_pendingFlats.empty() && m_pendingRecords.empty() && m_pendingMutations.empty() && m_pendingNames.empty();
}

void App::TweakChangeset::Commit(const Core::SharedPtr<Red::TweakDBManager>& aManager,
                                 const Core::SharedPtr<App::TweakChangelog>& aChangelog)
{
    if (!aManager)
        return;

    if (aChangelog)
    {
        aChangelog->RevertChanges(aManager);
        aChangelog->ForgetForeignKeys();
    }

    Core::Set<Red::TweakDBID> updates;
    Core::Map<Red::TweakDBID, Red::TweakDBID> associations;

    {
        const auto batch = aManager->StartBatch();

        for (const auto& item : m_pendingFlats)
        {
            const auto& flatId = item.first;
            const auto& flatType = item.second.type;
            const auto& flatValue = item.second.value.get();

            const auto success = aManager->SetFlat(batch, flatId, flatType, flatValue);

            if (!success)
            {
                LogError("Can't assign flat {}.", ToName(flatId));
                continue;
            }

            if (aChangelog)
            {
                if (aManager->GetReflection()->IsForeignKey(flatType))
                {
                    const auto foreignKey = reinterpret_cast<Red::TweakDBID*>(flatValue);
                    aChangelog->RegisterForeignKey(*foreignKey);
                }
                else if (aManager->GetReflection()->IsForeignKeyArray(flatType))
                {
                    const auto foreignKeyList = reinterpret_cast<Red::DynArray<Red::TweakDBID>*>(flatValue);
                    for (const auto& foreignKey : *foreignKeyList)
                    {
                        aChangelog->RegisterForeignKey(foreignKey);
                    }
                }
            }
        }

        for (const auto& recordId : m_orderedRecords)
        {
            const auto& recordEntry = m_pendingRecords[recordId];

            if (aManager->IsRecordExists(recordId))
            {
                updates.insert(recordId);
                continue;
            }

            if (recordEntry.sourceId.IsValid())
            {
                const auto success = aManager->CloneRecord(batch, recordId, recordEntry.sourceId);

                if (!success)
                {
                    LogError("Cannot clone record {} from {}.", ToName(recordId), ToName(recordEntry.sourceId));
                    continue;
                }
            }
            else
            {
                const auto success = aManager->CreateRecord(batch, recordId, recordEntry.type);

                if (!success)
                {
                    LogError("Cannot create record {} of type {}.", ToName(recordId), ToName(recordEntry.type));
                    continue;
                }
            }

            {
                const auto recordInfo = aManager->GetReflection()->GetRecordInfo(recordEntry.type);

                if (!recordInfo)
                {
                    LogWarning("Missing metadata for record {}.", ToName(recordId));
                    continue;
                }

                for (const auto& [_, propInfo] : recordInfo->props)
                {
                    if (propInfo->dataOffset)
                    {
                        associations.emplace(recordId + propInfo->appendix, recordId);
                    }
                }
            }
        }

        for (const auto& item : m_pendingNames)
        {
            aManager->RegisterName(batch, item.first, item.second);
        }

        aManager->CommitBatch(batch);
    }

    for (const auto& [flatId, mutation] : m_pendingMutations)
    {
        const auto& flatData = aManager->GetFlat(flatId);

        if (!flatData.value)
        {
            LogError("Cannot apply changes to [{}], the flat doesn't exist.", ToName(flatId));
            continue;
        }

        if (flatData.type->GetType() != Red::ERTTIType::Array)
        {
            LogError("Cannot apply changes to [{}], it's not an array.", ToName(flatId));
            continue;
        }

        auto* targetType = reinterpret_cast<Red::CRTTIArrayType*>(flatData.type);
        auto* elementType = targetType->innerType;

        // The data returned by manager is a pointer to the TweakDB flat buffer,
        // we must make a copy of the original array for modifications.
        auto targetArray = aManager->GetReflection()->Construct(targetType);
        targetType->Assign(targetArray.get(), flatData.value);

        Core::Vector<ElementChange> skips;
        Core::Vector<ElementChange> deletions;
        Core::Vector<ElementChange> insertions;
        Core::Vector<decltype(&mutation)> chain;

        {
            chain.push_back(&mutation);

            auto baseId = mutation.baseId;
            while (baseId.IsValid())
            {
                auto& entry = m_pendingMutations[baseId];
                chain.push_back(&entry);

                baseId = entry.baseId;
            }

            std::reverse(chain.begin(), chain.end());
        }

        {
            auto chainLevel = 0;

            for (const auto& entry : chain)
            {
                for (const auto& deletion : entry->deletions)
                {
                    const auto deletionValue = deletion.value;
                    const auto deletionIndex = FindElement(targetType, targetArray.get(), deletionValue.get());

                    if (deletionIndex >= 0)
                    {
                        deletions.emplace_back(deletionIndex, deletionValue);
                    }

                    skips.emplace_back(chainLevel, deletionValue);
                }

                ++chainLevel;
            }

            if (!deletions.empty())
            {
                auto descending = [](ElementChange& a, ElementChange& b) { return a.first > b.first; };
                std::sort(deletions.begin(), deletions.end(), descending);

                for (const auto& [deletionIndex, deletionEntry] : deletions)
                {
                    targetType->RemoveAt(targetArray.get(), deletionIndex);
                }
            }
        }

        {
            auto chainLevel = 0;
            auto insertionIndex = 0;

            auto performInsertions = [&, flatId = flatId](const Core::Vector<InsertionEntry>& aInsertions,
                                                          const Core::Vector<MergingEntry>& aMerges)
            {
                for (const auto& insertion : aInsertions)
                {
                    const auto& insertionValue = insertion.value;

                    if (insertion.unique && InArray(targetType, targetArray.get(), insertionValue.get()))
                        continue;

                    if (IsSkip(targetType, insertionValue.get(), chainLevel, skips))
                        continue;

                    targetType->InsertAt(targetArray.get(), insertionIndex);
                    elementType->Assign(targetType->GetElement(targetArray.get(), insertionIndex), insertionValue.get());

                    insertions.emplace_back(insertionIndex, insertionValue);

                    ++insertionIndex;
                }

                for (const auto& merge : aMerges)
                {
                    const auto sourceData = aManager->GetFlat(merge.sourceId);

                    if (!sourceData.value || sourceData.type != targetType)
                    {
                        LogError("Cannot merge [{}] with [{}] because it's not an array.",
                                 ToName(merge.sourceId), ToName(flatId));
                        continue;
                    }

                    auto* sourceArray = reinterpret_cast<Red::DynArray<void>*>(sourceData.value);
                    const auto sourceLength = targetType->GetLength(sourceArray);

                    for (uint32_t sourceIndex = 0; sourceIndex < sourceLength; ++sourceIndex)
                    {
                        const auto insertionValuePtr = targetType->GetElement(sourceArray, sourceIndex);

                        if (InArray(targetType, targetArray.get(), insertionValuePtr))
                            continue;

                        if (IsSkip(targetType, insertionValuePtr, chainLevel, skips))
                            continue;

                        targetType->InsertAt(targetArray.get(), insertionIndex);
                        elementType->Assign(targetType->GetElement(targetArray.get(), insertionIndex), insertionValuePtr);

                        auto clonedValue = aManager->GetReflection()->Construct(elementType);
                        elementType->Assign(clonedValue.get(), insertionValuePtr);

                        insertions.emplace_back(insertionIndex, clonedValue);

                        ++insertionIndex;
                    }
                }
            };

            chainLevel = 0;

            for (const auto& entry : chain)
            {
                performInsertions(entry->prependings, entry->prependingMerges);
                ++chainLevel;
            }

            chainLevel = 0;
            insertionIndex = static_cast<int32_t>(targetType->GetLength(targetArray.get()));

            for (const auto& entry : chain)
            {
                performInsertions(entry->appendings, entry->appendingMerges);
                ++chainLevel;
            }
        }

        const auto success = aManager->SetFlat(flatId, targetType, targetArray.get());

        if (!success)
        {
            LogError("Cannot assign flat value [{}].", ToName(flatId));
            continue;
        }

        {
            const auto association = associations.find(flatId);

            if (association != associations.end())
            {
                const auto& recordId = association.value();
                updates.insert(recordId);

                if (aChangelog)
                {
                    aChangelog->AssociateFlat(flatId, recordId);
                }
            }
        }

        if (aChangelog)
        {
            const auto isForeignKey = aManager->GetReflection()->IsForeignKeyArray(targetType);

            for (const auto& [deletionIndex, deletionValue] : deletions)
            {
                aChangelog->RegisterDeletion(flatId, deletionIndex, deletionValue);
            }

            for (const auto& [insertionIndex, insertionValue] : insertions)
            {
                aChangelog->RegisterInsertion(flatId, insertionIndex, insertionValue);

                if (isForeignKey)
                {
                    const auto foreignKey = reinterpret_cast<Red::TweakDBID*>(insertionValue.get());
                    aChangelog->RegisterForeignKey(*foreignKey);
                    aChangelog->RegisterName(*foreignKey, ToName(*foreignKey));
                }
            }

            aChangelog->RegisterName(flatId, ToName(flatId));
        }
    }

    for (const auto recordId : updates)
    {
        const auto success = aManager->UpdateRecord(recordId);

        if (!success)
        {
            LogError("Cannot update record [{}].", ToName(recordId));
        }
    }

    m_pendingFlats.clear();
    m_pendingRecords.clear();
    m_orderedRecords.clear();
    m_pendingNames.clear();
    m_pendingMutations.clear();
}

int32_t App::TweakChangeset::FindElement(Red::CRTTIArrayType* aArrayType, void* aArray, void* aValue)
{
    const auto length = aArrayType->GetLength(aArray);

    for (int32_t i = 0; i < length; ++i)
    {
        const auto element = aArrayType->GetElement(aArray, i);

        if (aArrayType->innerType->IsEqual(element, aValue))
        {
            return i;
        }
    }

    return -1;
}

bool App::TweakChangeset::InArray(Red::CRTTIArrayType* aArrayType, void* aArray, void* aValue)
{
    return FindElement(aArrayType, aArray, aValue) > 0;
}

bool App::TweakChangeset::IsSkip(Red::CRTTIArrayType* aArrayType, void* aValue, int32_t aLevel,
                                 const Core::Vector<ElementChange>& aChanges)
{
    for (const auto& [level, value] : aChanges)
    {
        if (level > aLevel && aArrayType->innerType->IsEqual(value.get(), aValue))
        {
            return true;
        }
    }

    return false;
}

std::string App::TweakChangeset::ToName(const Red::CBaseRTTIType* aType)
{
    return aType->GetName().ToString();
}

std::string App::TweakChangeset::ToName(Red::TweakDBID aId)
{
    const auto name = m_pendingNames.find(aId);

    if (name != m_pendingNames.end())
        return name.value();

    return fmt::format("<TDBID:{:08X}:{:02X}>", aId.name.hash, aId.name.length);
}
