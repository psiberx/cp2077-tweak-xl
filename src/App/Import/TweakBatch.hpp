#pragma once

#include "Core/Logging/LoggingAgent.hpp"
#include "TweakDB/Manager.hpp"

namespace App
{
class TweakBatch : Core::LoggingAgent
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

    enum class AlteringMethod
    {
        Append,
        // Prepend,
    };

    struct AlteringItem
    {
        AlteringMethod method{};
        const RED4ext::CBaseRTTIType* type;
        Core::SharedPtr<void> value;
        bool unique;
    };

    struct AlteringMerge
    {
        AlteringMethod method{};
        RED4ext::TweakDBID sourceId;
    };

    struct AlteringEntry
    {
        Core::Vector<AlteringItem> items;
        Core::Vector<AlteringMerge> merges;
    };

    explicit TweakBatch(TweakDB::Manager& aManager);

    bool SetFlat(RED4ext::TweakDBID aFlatId, const RED4ext::CBaseRTTIType* aType,
                 const Core::SharedPtr<void>& aValue);

    bool MakeRecord(RED4ext::TweakDBID aRecordId, const RED4ext::CClass* aType,
                    RED4ext::TweakDBID aSourceId = 0);

    bool AssociateRecord(RED4ext::TweakDBID aRecordId, RED4ext::CName aPropName,
                         RED4ext::TweakDBID aFlatId);

    bool AppendElement(RED4ext::TweakDBID aFlatId, const RED4ext::CBaseRTTIType* aType,
                       const Core::SharedPtr<void>& aValue, bool aUnique = false);

    bool AppendFrom(RED4ext::TweakDBID aFlatId, RED4ext::TweakDBID aSourceId);

    bool RegisterName(RED4ext::TweakDBID aId, const std::string& aName);

    bool IsEmpty();

    const FlatEntry* GetFlat(RED4ext::TweakDBID aFlatId);
    const RecordEntry* GetRecord(RED4ext::TweakDBID aRecordId);

    void Dispatch();

private:
    static bool InArray(RED4ext::CRTTIArrayType* aArrayType, RED4ext::ScriptInstance aArray,
                        RED4ext::ScriptInstance aValue);

    static std::string AsString(const RED4ext::CBaseRTTIType* aType);
    std::string AsString(RED4ext::TweakDBID aId);

    TweakDB::Manager& m_manager;
    Core::Map<RED4ext::TweakDBID, FlatEntry> m_pendingFlats;
    Core::Map<RED4ext::TweakDBID, RecordEntry> m_pendingRecords;
    Core::Vector<RED4ext::TweakDBID> m_orderedRecords;
    Core::Map<RED4ext::TweakDBID, AlteringEntry> m_pendingAlterings;
    Core::Map<RED4ext::TweakDBID, std::string> m_pendingNames;
    Core::Map<RED4ext::TweakDBID, RED4ext::TweakDBID> m_associationMap;
};
}
