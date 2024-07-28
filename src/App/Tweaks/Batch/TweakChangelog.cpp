#include "TweakChangelog.hpp"
#include "Red/TweakDB/Source/Source.hpp"

bool App::TweakChangelog::RegisterRecord(Red::TweakDBID aRecordId)
{
    if (!aRecordId.IsValid())
        return false;

    m_records.insert(aRecordId);

    return true;
}

bool App::TweakChangelog::RegisterAssignment(Red::TweakDBID aFlatId, Red::Instance aOldValue, Red::Instance aNewValue)
{
    if (!aFlatId.IsValid() || !aOldValue || !aNewValue || aOldValue == aNewValue)
        return false;

    aFlatId.SetTDBOffset(0);
    m_ownedKeys.insert(aFlatId);

    auto& entry = m_assignments[aFlatId];
    entry.previous = aOldValue;
    entry.current = aNewValue;

    return false;
}

bool App::TweakChangelog::RegisterInsertion(Red::TweakDBID aFlatId, int32_t aIndex, const Red::InstancePtr<>& aInstance)
{
    if (!aFlatId.IsValid() || !aInstance)
        return false;

    aFlatId.SetTDBOffset(0);
    m_ownedKeys.insert(aFlatId);

    auto& entry = m_mutations[aFlatId];
    entry.insertions[aIndex] = aInstance;

    return true;
}

bool App::TweakChangelog::RegisterDeletion(Red::TweakDBID aFlatId, int32_t aIndex, const Red::InstancePtr<>& aInstance)
{
    if (!aFlatId.IsValid() || !aInstance)
        return false;

    aFlatId.SetTDBOffset(0);
    m_ownedKeys.insert(aFlatId);

    auto& entry = m_mutations[aFlatId];
    entry.deletions[aIndex] = aInstance;

    return true;
}

void App::TweakChangelog::ForgetChanges(Red::TweakDBID aFlatId)
{
    if (!aFlatId.IsValid())
        return;

    aFlatId.SetTDBOffset(0);

    m_mutations.erase(aFlatId);
}

void App::TweakChangelog::RegisterForeignKey(Red::TweakDBID aForeignKey, Red::TweakDBID aFlatId)
{
    if (aForeignKey.IsValid())
    {
        m_foreignKeys[aForeignKey] = aFlatId;
    }
}

void App::TweakChangelog::ForgetForeignKey(Red::TweakDBID aForeignKey)
{
    if (aForeignKey.IsValid())
    {
        m_foreignKeys.erase(aForeignKey);
    }
}

void App::TweakChangelog::ForgetForeignKeys()
{
    m_foreignKeys.clear();
}

void App::TweakChangelog::RegisterResourcePath(Red::ResourcePath aPath, Red::TweakDBID aFlatId)
{
    if (aPath)
    {
        m_resourcePaths[aPath] = aFlatId;
    }
}

void App::TweakChangelog::ForgetResourcePath(Red::ResourcePath aPath)
{
    if (aPath)
    {
        m_resourcePaths.erase(aPath);
    }
}

void App::TweakChangelog::ForgetResourcePaths()
{
    m_resourcePaths.clear();
}

void App::TweakChangelog::CheckForIssues(const Core::SharedPtr<Red::TweakDBManager>& aManager)
{
    {
        Core::Map<Red::TweakDBID, Core::Set<Red::TweakDBID>> brokenRefs;

        for (const auto& [foreignKey, flatId] : m_foreignKeys)
        {
            if (!aManager->IsRecordExists(foreignKey) && !aManager->IsFlatExists(foreignKey))
            {
                brokenRefs[flatId].insert(foreignKey);
            }
        }

        for (const auto& [flatId, foreignKeys] : brokenRefs)
        {
            const auto& flatName = aManager->GetName(flatId);
            if (!flatName.starts_with(Red::TweakSource::SchemaPackage))
            {
                for (const auto& foreignKey : foreignKeys)
                {
                    const auto& foreignKeyName = aManager->GetName(foreignKey);
                    LogWarning("{} refers to a non-existent record or flat {}.", flatName, foreignKeyName);
                }
            }
        }
    }

    {
        Core::Set<Red::TweakDBID> brokenRefIds;

        auto depot = Red::ResourceDepot::Get();
        for (const auto& [resourcePath, flatId] : m_resourcePaths)
        {
            if (!depot->ResourceExists(resourcePath))
            {
                brokenRefIds.insert(flatId);
            }
        }

        for (const auto& flatId : brokenRefIds)
        {
            LogWarning("{} refers to a non-existent resource.", aManager->GetName(flatId));
        }
    }
}

void App::TweakChangelog::RevertChanges(const Core::SharedPtr<Red::TweakDBManager>& aManager)
{
    Core::Set<Red::TweakDBID> updates;

    for (const auto& [flatId, mutation] : m_mutations)
    {
        const auto& flatData = aManager->GetFlat(flatId);

        if (!flatData.instance)
        {
            LogWarning("Cannot restore {}, the flat doesn't exist.", aManager->GetName(flatId));
            continue;
        }

        if (flatData.type->GetType() != Red::ERTTIType::Array)
        {
            LogWarning("Cannot restore {}, it's not an array.", aManager->GetName(flatId));
            continue;
        }

        auto arrayType = reinterpret_cast<const Red::CRTTIArrayType*>(flatData.type);
        auto elementType = arrayType->innerType;
        auto canRestore = true;

        {
            auto currentArray = flatData.instance;
            auto currentSize = arrayType->GetLength(currentArray);

            for (const auto& [insertionIndex, insertionValue] : mutation.insertions)
            {
                if (insertionIndex >= currentSize)
                {
                    canRestore = false;
                    break;
                }

                auto currentElement = arrayType->GetElement(currentArray, insertionIndex);

                if (!elementType->IsEqual(currentElement, insertionValue.get()))
                {
                    canRestore = false;
                    break;
                }
            }

            currentSize -= mutation.insertions.size();
            currentSize += mutation.deletions.size();

            for (const auto& [deletionIndex, deletionValue] : mutation.deletions)
            {
                if (deletionIndex >= currentSize)
                {
                    canRestore = false;
                    break;
                }
            }
        }

        if (!canRestore)
        {
            LogWarning("Cannot restore {}, third party changes detected.", aManager->GetName(flatId));
            continue;
        }

        auto restoredArray = aManager->GetReflection()->Construct(arrayType);
        arrayType->Assign(restoredArray.get(), flatData.instance);

        for (const auto& [insertionIndex, insertionValue] : mutation.insertions | std::views::reverse)
        {
            arrayType->RemoveAt(restoredArray.get(), insertionIndex);
        }

        for (const auto& [deletionIndex, deletionValue] : mutation.deletions)
        {
            arrayType->InsertAt(restoredArray.get(), deletionIndex);
            elementType->Assign(arrayType->GetElement(restoredArray.get(), deletionIndex), deletionValue.get());
        }

        const auto success = aManager->SetFlat(flatId, arrayType, restoredArray.get());

        if (!success)
        {
            LogError("Cannot restore {}, failed to assign the value.", aManager->GetName(flatId));
            continue;
        }
    }

    for (const auto& [flatId, assignment] : m_assignments)
    {
        const auto& flatData = aManager->GetFlat(flatId);

        if (!flatData.instance)
        {
            LogWarning("Cannot restore {}, the flat doesn't exist.", aManager->GetName(flatId));
            continue;
        }

        if (!m_ownedKeys.contains(flatId) && flatData.instance != assignment.current)
        {
            LogWarning("Cannot restore {}, third party changes detected.", aManager->GetName(flatId));
            continue;
        }

        const auto success = aManager->SetFlat(flatId, flatData.type, assignment.previous);

        if (!success)
        {
            LogError("Cannot restore {}, failed to assign the value.", aManager->GetName(flatId));
            continue;
        }
    }

    for (const auto recordId : m_records)
    {
        const auto success = aManager->UpdateRecord(recordId);

        if (!success)
        {
            LogError("Cannot restore {}, failed to update the record.", aManager->GetName(recordId));
        }
    }

    m_records.clear();
    m_assignments.clear();
    m_mutations.clear();
}

const Core::Set<Red::TweakDBID>& App::TweakChangelog::GetAffectedRecords() const
{
    return m_records;
}
