#pragma once

#include "App/Tweaks/Batch/TweakChangelog.hpp"
#include "Core/Logging/LoggingAgent.hpp"
#include "Red/TweakDB/Manager.hpp"

namespace App
{
class TweakChangeset : public Core::LoggingAgent
{
public:
    struct FlatEntry
    {
        const Red::CBaseRTTIType* type;
        Core::SharedPtr<void> value;
    };

    struct RecordEntry
    {
        const Red::CClass* type;
        Red::TweakDBID sourceId;
    };

    struct InsertionEntry
    {
        const Red::CBaseRTTIType* type;
        Core::SharedPtr<void> value;
        bool unique;
    };

    struct DeletionEntry
    {
        const Red::CBaseRTTIType* type;
        Core::SharedPtr<void> value;
    };

    struct MergingEntry
    {
        Red::TweakDBID sourceId;
    };

    struct MutationEntry
    {
        Red::TweakDBID baseId;
        Core::Vector<DeletionEntry> deletions;
        Core::Vector<InsertionEntry> appendings;
        Core::Vector<InsertionEntry> prependings;
        Core::Vector<MergingEntry> appendingMerges;
        Core::Vector<MergingEntry> prependingMerges;
    };

    bool SetFlat(Red::TweakDBID aFlatId, const Red::CBaseRTTIType* aType,
                 const Core::SharedPtr<void>& aValue);

    bool MakeRecord(Red::TweakDBID aRecordId, const Red::CClass* aType,
                    Red::TweakDBID aSourceId = 0);
    bool UpdateRecord(Red::TweakDBID aRecordId);

    bool AppendElement(Red::TweakDBID aFlatId, const Red::CBaseRTTIType* aType,
                       const Core::SharedPtr<void>& aValue, bool aUnique = false);
    bool PrependElement(Red::TweakDBID aFlatId, const Red::CBaseRTTIType* aType,
                        const Core::SharedPtr<void>& aValue, bool aUnique = false);
    bool RemoveElement(Red::TweakDBID aFlatId, const Red::CBaseRTTIType* aType,
                       const Core::SharedPtr<void>& aValue);
    bool AppendFrom(Red::TweakDBID aFlatId, Red::TweakDBID aSourceId);
    bool PrependFrom(Red::TweakDBID aFlatId, Red::TweakDBID aSourceId);
    bool InheritChanges(Red::TweakDBID aFlatId, Red::TweakDBID aBaseId);

    bool RegisterName(Red::TweakDBID aId, const std::string& aName);

    const FlatEntry* GetFlat(Red::TweakDBID aFlatId);
    const RecordEntry* GetRecord(Red::TweakDBID aRecordId);
    bool HasRecord(Red::TweakDBID aRecordId);

    bool IsEmpty();

    void Commit(const Core::SharedPtr<Red::TweakDBManager>& aManager,
                const Core::SharedPtr<App::TweakChangelog>& aChangelog);

private:
    using ElementChange = std::pair<int32_t, Core::SharedPtr<void>>;

    static int32_t FindElement(Red::CRTTIArrayType* aArrayType, void* aArray, void* aValue);
    static bool InArray(Red::CRTTIArrayType* aArrayType, void* aArray, void* aValue);
    static bool IsSkip(Red::CRTTIArrayType* aArrayType, void* aValue, int32_t aLevel,
                       const Core::Vector<ElementChange>& aChanges);

    static std::string ToName(const Red::CBaseRTTIType* aType);
    std::string ToName(Red::TweakDBID aId);

    Core::Map<Red::TweakDBID, FlatEntry> m_pendingFlats;
    Core::Map<Red::TweakDBID, RecordEntry> m_pendingRecords;
    Core::Vector<Red::TweakDBID> m_orderedRecords;
    Core::Map<Red::TweakDBID, MutationEntry> m_pendingMutations;
    Core::Map<Red::TweakDBID, std::string> m_pendingNames;
};
}
