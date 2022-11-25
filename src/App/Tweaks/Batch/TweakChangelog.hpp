#pragma once

#include "Core/Logging/LoggingAgent.hpp"
#include "Red/TweakDB/Manager.hpp"

namespace App
{
class TweakChangelog : public Core::LoggingAgent
{
public:
    bool RegisterRecord(Red::TweakDBID aRecordId);

    bool RegisterAssignment(Red::TweakDBID aFlatId, Red::Instance aOldValue, Red::Instance aNewValue);
    bool RegisterInsertion(Red::TweakDBID aFlatId, int32_t aIndex, const Red::InstancePtr<>& aInstance);
    bool RegisterDeletion(Red::TweakDBID aFlatId, int32_t aIndex, const Red::InstancePtr<>& aInstance);
    void ForgetChanges(Red::TweakDBID aFlatId);

    void RegisterForeignKey(Red::TweakDBID aForeignKey);
    void ForgetForeignKey(Red::TweakDBID aForeignKey);
    void ForgetForeignKeys();

    void RegisterName(Red::TweakDBID aId, const std::string& aName);

    void CheckForIssues(const Core::SharedPtr<Red::TweakDBManager>& aManager);
    void RevertChanges(const Core::SharedPtr<Red::TweakDBManager>& aManager);

private:
    struct AssignmentEntry
    {
        Red::Instance previous;
        Red::Instance current;
    };

    struct MutationEntry
    {
        Core::SortedMap<int32_t, Red::InstancePtr<>> insertions;
        Core::SortedMap<int32_t, Red::InstancePtr<>> deletions;
    };

    inline bool IsOwnKey(Red::TweakDBID aId);
    std::string ToName(Red::TweakDBID aId);

    Core::Set<Red::TweakDBID> m_records;
    Core::Map<Red::TweakDBID, AssignmentEntry> m_assignments;
    Core::Map<Red::TweakDBID, MutationEntry> m_mutations;
    Core::Map<Red::TweakDBID, std::string> m_knownNames;
    Core::Set<Red::TweakDBID> m_foreignKeys;
    Core::Set<Red::TweakDBID> m_ownedKeys;
};
}
