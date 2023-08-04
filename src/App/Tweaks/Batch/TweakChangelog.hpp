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

    void RegisterForeignKey(Red::TweakDBID aForeignKey, Red::TweakDBID aFlatId);
    void ForgetForeignKey(Red::TweakDBID aForeignKey);
    void ForgetForeignKeys();

    void RegisterResourcePath(Red::ResourcePath aPath, Red::TweakDBID aFlatId);
    void ForgetResourcePath(Red::ResourcePath aPath);
    void ForgetResourcePaths();

    void CheckForIssues(const Core::SharedPtr<Red::TweakDBManager>& aManager);
    void RevertChanges(const Core::SharedPtr<Red::TweakDBManager>& aManager);

    [[nodiscard]] const Core::Set<Red::TweakDBID>& GetAffectedRecords() const;

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

    Core::Set<Red::TweakDBID> m_records;
    Core::Map<Red::TweakDBID, AssignmentEntry> m_assignments;
    Core::Map<Red::TweakDBID, MutationEntry> m_mutations;
    Core::Map<Red::TweakDBID, Red::TweakDBID> m_foreignKeys;
    Core::Map<Red::ResourcePath, Red::TweakDBID> m_resourcePaths;
    Core::Set<Red::TweakDBID> m_ownedKeys;
};
}
