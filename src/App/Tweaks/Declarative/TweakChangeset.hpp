#pragma once

#include "App/Tweaks/Declarative/TweakChangelog.hpp"
#include "Core/Logging/LoggingAgent.hpp"
#include "Red/TweakDB/Manager.hpp"

namespace App
{
class TweakChangeset : public Core::LoggingAgent
{
public:
    struct FlatEntry
    {
        const RED4ext::CBaseRTTIType* type;
        Core::SharedPtr<void> value;
    };

    struct RecordEntry
    {
        const RED4ext::CClass* type;
        RED4ext::TweakDBID sourceId;
    };

    struct InsertionEntry
    {
        const RED4ext::CBaseRTTIType* type;
        Core::SharedPtr<void> value;
        bool unique;
    };

    struct DeletionEntry
    {
        const RED4ext::CBaseRTTIType* type;
        Core::SharedPtr<void> value;
    };

    struct MergingEntry
    {
        RED4ext::TweakDBID sourceId;
    };

    struct AlteringEntry
    {
        Core::Vector<DeletionEntry> deletions;
        Core::Vector<InsertionEntry> appendings;
        Core::Vector<InsertionEntry> prependings;
        Core::Vector<MergingEntry> appendingMerges;
        Core::Vector<MergingEntry> prependingMerges;
    };

    bool SetFlat(RED4ext::TweakDBID aFlatId, const RED4ext::CBaseRTTIType* aType,
                 const Core::SharedPtr<void>& aValue);

    bool MakeRecord(RED4ext::TweakDBID aRecordId, const RED4ext::CClass* aType,
                    RED4ext::TweakDBID aSourceId = 0);
    bool UpdateRecord(RED4ext::TweakDBID aRecordId);
    bool AssociateRecord(RED4ext::TweakDBID aRecordId, RED4ext::TweakDBID aFlatId);

    bool AppendElement(RED4ext::TweakDBID aFlatId, const RED4ext::CBaseRTTIType* aType,
                       const Core::SharedPtr<void>& aValue, bool aUnique = false);
    bool PrependElement(RED4ext::TweakDBID aFlatId, const RED4ext::CBaseRTTIType* aType,
                        const Core::SharedPtr<void>& aValue, bool aUnique = false);
    bool RemoveElement(RED4ext::TweakDBID aFlatId, const RED4ext::CBaseRTTIType* aType,
                       const Core::SharedPtr<void>& aValue);
    bool AppendFrom(RED4ext::TweakDBID aFlatId, RED4ext::TweakDBID aSourceId);
    bool PrependFrom(RED4ext::TweakDBID aFlatId, RED4ext::TweakDBID aSourceId);
    bool InheritChanges(RED4ext::TweakDBID aFlatId, RED4ext::TweakDBID aBaseId);

    bool RegisterName(RED4ext::TweakDBID aId, const std::string& aName);

    const FlatEntry* GetFlat(RED4ext::TweakDBID aFlatId);
    const RecordEntry* GetRecord(RED4ext::TweakDBID aRecordId);
    bool HasRecord(RED4ext::TweakDBID aRecordId);

    bool IsEmpty();

    void Commit(Core::SharedPtr<Red::TweakDB::Manager>& aManager, Core::SharedPtr<TweakChangelog>& aChangelog);

private:
    using ElementChange = std::pair<int32_t, Core::SharedPtr<void>>;

    struct InsertionHandler
    {
        RED4ext::TweakDBID m_arrayId;
        RED4ext::CRTTIArrayType* m_arrayType;
        RED4ext::CBaseRTTIType* m_elementType;
        Core::SharedPtr<void>& m_array;
        Core::Vector<ElementChange>& m_changes;
        Core::SharedPtr<Red::TweakDB::Manager>& m_manager;
        TweakChangeset& m_changeset;

        void Apply(const Core::Vector<InsertionEntry>& aInsertions, const Core::Vector<MergingEntry>& aMerges,
                   int32_t aStargIndex);
    };

    static int32_t FindElement(RED4ext::CRTTIArrayType* aArrayType, void* aArray, void* aValue);
    static bool InArray(RED4ext::CRTTIArrayType* aArrayType, void* aArray, void* aValue);

    static std::string AsString(const RED4ext::CBaseRTTIType* aType);
    std::string AsString(RED4ext::TweakDBID aId);

    Core::Map<RED4ext::TweakDBID, FlatEntry> m_pendingFlats;
    Core::Map<RED4ext::TweakDBID, RecordEntry> m_pendingRecords;
    Core::Vector<RED4ext::TweakDBID> m_orderedRecords;
    Core::Map<RED4ext::TweakDBID, AlteringEntry> m_pendingAlterings;
    Core::Map<RED4ext::TweakDBID, std::string> m_pendingNames;
    Core::Map<RED4ext::TweakDBID, RED4ext::TweakDBID> m_flatToRecordMap;
};
}
