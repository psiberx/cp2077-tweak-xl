#pragma once

#include "Core/Logging/LoggingAgent.hpp"
#include "Red/TweakDB/Manager.hpp"

namespace App
{
class TweakChangelog : public Core::LoggingAgent
{
public:
    bool AssociateFlat(Red::TweakDBID aFlatId, Red::TweakDBID aRecordId);
    bool RegisterInsertion(Red::TweakDBID aFlatId, int32_t aIndex, const Core::SharedPtr<void>& aValue);
    bool RegisterDeletion(Red::TweakDBID aFlatId, int32_t aIndex, const Core::SharedPtr<void>& aValue);
    void ForgetChanges(Red::TweakDBID aFlatId);

    void RegisterForeignKey(Red::TweakDBID aForeignKey);
    void ForgetForeignKey(Red::TweakDBID aForeignKey);
    void ForgetForeignKeys();

    void RegisterName(Red::TweakDBID aId, const std::string& aName);

    void CheckForIssues(const Core::SharedPtr<Red::TweakDBManager>& aManager);
    void RevertChanges(const Core::SharedPtr<Red::TweakDBManager>& aManager);

private:
    struct MutationEntry
    {
        Red::TweakDBID recordId;
        Core::SortedMap<int32_t, Core::SharedPtr<void>> insertions;
        Core::SortedMap<int32_t, Core::SharedPtr<void>> deletions;
    };

    std::string AsString(Red::TweakDBID aId);

    Core::Map<Red::TweakDBID, MutationEntry> m_mutations;
    Core::Map<Red::TweakDBID, std::string> m_knownNames;
    Core::Set<Red::TweakDBID> m_foreignKeys;
};
}