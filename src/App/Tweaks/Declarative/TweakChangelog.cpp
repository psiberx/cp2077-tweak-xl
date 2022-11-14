#include "TweakChangelog.hpp"

bool App::TweakChangelog::AssociateRecord(RED4ext::TweakDBID aRecordId, RED4ext::TweakDBID aFlatId)
{
    if (!aRecordId.IsValid() || !aFlatId.IsValid())
        return false;

    m_alterings[aFlatId].recordId = aRecordId;

    return true;
}

bool App::TweakChangelog::RegisterInsertion(RED4ext::TweakDBID aFlatId, int32_t aIndex,
                                            const Core::SharedPtr<void>& aValue)
{
    if (!aFlatId.IsValid() || !aValue)
        return false;

    auto& entry = m_alterings[aFlatId];
    entry.insertions[aIndex] = aValue;

    return true;
}

bool App::TweakChangelog::RegisterDeletion(RED4ext::TweakDBID aFlatId, int32_t aIndex,
                                           const Core::SharedPtr<void>& aValue)
{
    if (!aFlatId.IsValid() || !aValue)
        return false;

    auto& entry = m_alterings[aFlatId];
    entry.deletions[aIndex] = aValue;

    return true;
}

void App::TweakChangelog::ForgetChanges(RED4ext::TweakDBID aFlatId)
{
    m_alterings.erase(aFlatId);
}

void App::TweakChangelog::RegisterForeignKey(RED4ext::TweakDBID aForeignKey)
{
    m_foreignKeys.insert(aForeignKey);
}

void App::TweakChangelog::ForgetForeignKey(RED4ext::TweakDBID aForeignKey)
{
    m_foreignKeys.erase(aForeignKey);
}

void App::TweakChangelog::ForgetForeignKeys()
{
    m_foreignKeys.clear();
}

void App::TweakChangelog::RegisterName(RED4ext::TweakDBID aId, const std::string& aName)
{
    m_knownNames[aId] = aName;
}

void App::TweakChangelog::CheckForIssues(Core::SharedPtr<Red::TweakDB::Manager>& aManager)
{
    for (const auto& foreignKey : m_foreignKeys)
    {
        if (!aManager->IsRecordExists(foreignKey) && !aManager->IsFlatExists(foreignKey))
        {
            LogWarning("Foreign key [{}] refers to a non-existent record or flat.", AsString(foreignKey));
        }
    }
}

void App::TweakChangelog::RevertChanges(Core::SharedPtr<Red::TweakDB::Manager>& aManager)
{
    for (const auto& [flatId, altering] : m_alterings)
    {
        const auto& flatData = aManager->GetFlat(flatId);

        if (!flatData.value)
        {
            LogWarning("Cannot restore [{}], the flat doesn't exist.", AsString(flatId));
            continue;
        }

        if (flatData.type->GetType() != RED4ext::ERTTIType::Array)
        {
            LogWarning("Cannot restore [{}], it's not an array.", AsString(flatId));
            continue;
        }

        auto arrayType = reinterpret_cast<RED4ext::CRTTIArrayType*>(flatData.type);
        auto elementType = arrayType->innerType;
        auto canRestore = true;

        {
            auto currentArray = flatData.value;
            auto currentSize = arrayType->GetLength(currentArray);

            for (const auto& [insertionIndex, insertionValue] : altering.insertions)
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

            currentSize -= altering.insertions.size();
            currentSize += altering.deletions.size();

            for (const auto& [deletionIndex, deletionValue] : altering.deletions)
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

        auto restoredArray = Red::TweakDB::MakeDefault(arrayType);
        arrayType->Assign(restoredArray.get(), flatData.value);

        for (const auto& [insertionIndex, insertionValue] : altering.insertions | std::views::reverse)
        {
            arrayType->RemoveAt(restoredArray.get(), insertionIndex);
        }

        for (const auto& [deletionIndex, deletionValue] : altering.deletions)
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

        if (altering.recordId.IsValid())
        {
            aManager->UpdateRecord(altering.recordId);
        }
    }

    m_alterings.clear();
}

std::string App::TweakChangelog::AsString(RED4ext::TweakDBID aId)
{
    const auto name = m_knownNames.find(aId);

    if (name != m_knownNames.end())
        return name.value();

    return fmt::format("<TDBID:{:08X}:{:02X}>", aId.name.hash, aId.name.length);
}
