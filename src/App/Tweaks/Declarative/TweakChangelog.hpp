#pragma once

#include "Core/Logging/LoggingAgent.hpp"
#include "Red/TweakDB/Manager.hpp"

namespace App
{
class TweakChangelog : public Core::LoggingAgent
{
public:
    bool AssociateRecord(Red::TweakDBID aRecordId, Red::TweakDBID aFlatId);
    bool RegisterInsertion(Red::TweakDBID aFlatId, int32_t aIndex, const Core::SharedPtr<void>& aValue);
    bool RegisterDeletion(Red::TweakDBID aFlatId, int32_t aIndex, const Core::SharedPtr<void>& aValue);
    void ForgetChanges(Red::TweakDBID aFlatId);

    void RegisterForeignKey(Red::TweakDBID aForeignKey);
    void ForgetForeignKey(Red::TweakDBID aForeignKey);
    void ForgetForeignKeys();

    void RegisterName(Red::TweakDBID aId, const std::string& aName);

    void CheckForIssues(Core::SharedPtr<Red::TweakDBManager>& aManager);
    void RevertChanges(Core::SharedPtr<Red::TweakDBManager>& aManager);

private:
    struct AlteringEntry
    {
        Red::TweakDBID recordId;
        Core::SortedMap<int32_t, Core::SharedPtr<void>> insertions;
        Core::SortedMap<int32_t, Core::SharedPtr<void>> deletions;
    };

    std::string AsString(Red::TweakDBID aId);

    Core::Map<Red::TweakDBID, AlteringEntry> m_alterings;
    Core::Map<Red::TweakDBID, std::string> m_knownNames;
    Core::Set<Red::TweakDBID> m_foreignKeys;
};
}
