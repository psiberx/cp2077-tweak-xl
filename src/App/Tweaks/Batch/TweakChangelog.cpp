#include "TweakChangelog.hpp"

bool App::TweakChangelog::AssociateFlat(Red::TweakDBID aFlatId, Red::TweakDBID aRecordId)
{
    if (!aRecordId.IsValid() || !aFlatId.IsValid())
        return false;

    m_mutations[aFlatId].recordId = aRecordId;

    return true;
}

bool App::TweakChangelog::RegisterInsertion(Red::TweakDBID aFlatId, int32_t aIndex,
                                            const Core::SharedPtr<void>& aValue)
{
    if (!aFlatId.IsValid() || !aValue)
        return false;

    auto& entry = m_mutations[aFlatId];
    entry.insertions[aIndex] = aValue;

    return true;
}

bool App::TweakChangelog::RegisterDeletion(Red::TweakDBID aFlatId, int32_t aIndex,
                                           const Core::SharedPtr<void>& aValue)
{
    if (!aFlatId.IsValid() || !aValue)
        return false;

    auto& entry = m_mutations[aFlatId];
    entry.deletions[aIndex] = aValue;

    return true;
}

void App::TweakChangelog::ForgetChanges(Red::TweakDBID aFlatId)
{
    if (aFlatId.IsValid())
    {
        m_mutations.erase(aFlatId);
    }
}

void App::TweakChangelog::RegisterForeignKey(Red::TweakDBID aForeignKey)
{
    if (aForeignKey.IsValid())
    {
        m_foreignKeys.insert(aForeignKey);
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

void App::TweakChangelog::RegisterName(Red::TweakDBID aId, const std::string& aName)
{
    m_knownNames[aId] = aName;
}

void App::TweakChangelog::CheckForIssues(const Core::SharedPtr<Red::TweakDBManager>& aManager)
{
    for (const auto& foreignKey : m_foreignKeys)
    {
        if (!aManager->IsRecordExists(foreignKey) && !aManager->IsFlatExists(foreignKey))
        {
            LogWarning("Foreign key [{}] refers to a non-existent record or flat.", AsString(foreignKey));
        }
    }
}

void App::TweakChangelog::RevertChanges(const Core::SharedPtr<Red::TweakDBManager>& aManager)
{
    for (const auto& [flatId, mutation] : m_mutations)
    {
        const auto& flatData = aManager->GetFlat(flatId);

        if (!flatData.value)
        {
            LogWarning("Cannot restore [{}], the flat doesn't exist.", AsString(flatId));
            continue;
        }

        if (flatData.type->GetType() != Red::ERTTIType::Array)
        {
            LogWarning("Cannot restore [{}], it's not an array.", AsString(flatId));
            continue;
        }

        auto arrayType = reinterpret_cast<Red::CRTTIArrayType*>(flatData.type);
        auto elementType = arrayType->innerType;
        auto canRestore = true;

        {
            auto currentArray = flatData.value;
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
            LogWarning("Cannot restore [{}], third party changes detected.", AsString(flatId));
            continue;
        }

        auto restoredArray = aManager->GetReflection()->Construct(arrayType);
        arrayType->Assign(restoredArray.get(), flatData.value);

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
            LogError("Cannot restore [{}], failed to assign the value.", AsString(flatId));
            continue;
        }

        if (mutation.recordId.IsValid())
        {
            aManager->UpdateRecord(mutation.recordId);
        }
    }

    m_mutations.clear();
}

std::string App::TweakChangelog::AsString(Red::TweakDBID aId)
{
    const auto name = m_knownNames.find(aId);

    if (name != m_knownNames.end())
        return name.value();

    return fmt::format("<TDBID:{:08X}:{:02X}>", aId.name.hash, aId.name.length);
}
