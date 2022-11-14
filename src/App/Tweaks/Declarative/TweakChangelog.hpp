#pragma once

#include "Core/Logging/LoggingAgent.hpp"
#include "Red/TweakDB/Manager.hpp"

namespace App
{
class TweakChangelog : public Core::LoggingAgent
{
public:
    bool AssociateRecord(RED4ext::TweakDBID aRecordId, RED4ext::TweakDBID aFlatId);
    bool RegisterInsertion(RED4ext::TweakDBID aFlatId, int32_t aIndex, const Core::SharedPtr<void>& aValue);
    bool RegisterDeletion(RED4ext::TweakDBID aFlatId, int32_t aIndex, const Core::SharedPtr<void>& aValue);
    void ForgetChanges(RED4ext::TweakDBID aFlatId);

    void RegisterForeignKey(RED4ext::TweakDBID aForeignKey);
    void ForgetForeignKey(RED4ext::TweakDBID aForeignKey);
    void ForgetForeignKeys();

    void RegisterName(RED4ext::TweakDBID aId, const std::string& aName);

    void CheckForIssues(Core::SharedPtr<Red::TweakDB::Manager>& aManager);
    void RevertChanges(Core::SharedPtr<Red::TweakDB::Manager>& aManager);

private:
    struct AlteringEntry
    {
        RED4ext::TweakDBID recordId;
        Core::SortedMap<int32_t, Core::SharedPtr<void>> insertions;
        Core::SortedMap<int32_t, Core::SharedPtr<void>> deletions;
    };

    std::string AsString(RED4ext::TweakDBID aId);

    Core::Map<RED4ext::TweakDBID, AlteringEntry> m_alterings;
    Core::Map<RED4ext::TweakDBID, std::string> m_knownNames;
    Core::Set<RED4ext::TweakDBID> m_foreignKeys;
};
}
