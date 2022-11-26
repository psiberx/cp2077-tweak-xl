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
        Red::InstancePtr<> value;
    };

    struct RecordEntry
    {
        const Red::CClass* type;
        Red::TweakDBID sourceId;
    };

    struct InsertionEntry
    {
        const Red::CBaseRTTIType* type;
        Red::InstancePtr<> value;
        bool unique;
    };

    struct DeletionEntry
    {
        const Red::CBaseRTTIType* type;
        Red::InstancePtr<> value;
    };

    struct MergingEntry
    {
        Red::TweakDBID sourceId;
    };

    struct MutationEntry
    {
        Core::Vector<DeletionEntry> deletions;
        Core::Vector<InsertionEntry> appendings;
        Core::Vector<InsertionEntry> prependings;
        Core::Vector<MergingEntry> appendingMerges;
        Core::Vector<MergingEntry> prependingMerges;
        Red::TweakDBID baseId;
    };

    bool SetFlat(Red::TweakDBID aFlatId, const Red::CBaseRTTIType* aType,
                 const Red::InstancePtr<>& aValue);

    bool MakeRecord(Red::TweakDBID aRecordId, const Red::CClass* aType,
                    Red::TweakDBID aSourceId = 0);
    bool UpdateRecord(Red::TweakDBID aRecordId);

    bool AppendElement(Red::TweakDBID aFlatId, const Red::CBaseRTTIType* aType,
                       const Red::InstancePtr<>& aValue, bool aUnique = false);
    bool PrependElement(Red::TweakDBID aFlatId, const Red::CBaseRTTIType* aType,
                        const Red::InstancePtr<>& aValue, bool aUnique = false);
    bool RemoveElement(Red::TweakDBID aFlatId, const Red::CBaseRTTIType* aType,
                       const Red::InstancePtr<>& aValue);
    bool AppendFrom(Red::TweakDBID aFlatId, Red::TweakDBID aSourceId);
    bool PrependFrom(Red::TweakDBID aFlatId, Red::TweakDBID aSourceId);
    bool InheritMutations(Red::TweakDBID aFlatId, Red::TweakDBID aBaseId);

    bool RegisterName(Red::TweakDBID aId, const std::string& aName);

    const FlatEntry* GetFlat(Red::TweakDBID aFlatId);
    const RecordEntry* GetRecord(Red::TweakDBID aRecordId);
    const Red::CClass* GetRecordType(Red::TweakDBID aRecordId);
    bool HasRecord(Red::TweakDBID aRecordId);

    bool IsEmpty();

    void Commit(const Core::SharedPtr<Red::TweakDBManager>& aManager,
                const Core::SharedPtr<App::TweakChangelog>& aChangelog);

private:
    using ElementChange = std::pair<int32_t, Core::SharedPtr<void>>;

    static int32_t FindElement(const Red::CRTTIArrayType* aArrayType, void* aArray, void* aValue);
    static bool InArray(const Red::CRTTIArrayType* aArrayType, void* aArray, void* aValue);
    static bool IsSkip(const Red::CRTTIArrayType* aArrayType, void* aValue, int32_t aLevel,
                       const Core::Vector<ElementChange>& aChanges);

    Core::Vector<Red::TweakDBID> m_orderedRecords;
    Core::Map<Red::TweakDBID, RecordEntry> m_pendingRecords;
    Core::Map<Red::TweakDBID, MutationEntry> m_pendingMutations;
    Core::Map<Red::TweakDBID, FlatEntry> m_pendingFlats;
    Core::Map<Red::TweakDBID, std::string> m_pendingNames;
};
}
