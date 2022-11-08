#pragma once

#include "Alias.hpp"
#include "FlatPool.hpp"
#include "Reflection.hpp"
#include "Types.hpp"

namespace Red::TweakDB
{
class Manager
{
public:
    Manager();
    explicit Manager(Instance* aTweakDb);

    Manager(const Manager&) = delete;
    Manager& operator=(const Manager&) = delete;

    Red::CStackType GetFlat(Red::TweakDBID aFlatId);
    Red::Handle<Red::gamedataTweakDBRecord> GetRecord(Red::TweakDBID aRecordId);

    bool IsFlatExists(Red::TweakDBID aFlatId);
    bool IsRecordExists(Red::TweakDBID aRecordId);

    bool SetFlat(Red::TweakDBID aFlatId, const Red::CBaseRTTIType* aType, Red::ScriptInstance aValue);
    bool SetFlat(Red::TweakDBID aFlatId, Red::CStackType aData);

    bool CreateRecord(Red::TweakDBID aRecordId, const Red::CClass* aType);
    bool CloneRecord(Red::TweakDBID aRecordId, Red::TweakDBID aSourceId);
    bool UpdateRecord(Red::TweakDBID aRecordId);

    void RegisterName(Red::TweakDBID aId, const std::string& aName);

    void StartBatch();
    void CommitBatch();

    Reflection& GetReflection();

private:
    Instance* m_tweakDb;
    FlatPool m_flatPool;
    Reflection m_reflection;
    bool m_batchMode;
    Red::SortedUniqueArray<Red::TweakDBID> m_batchFlats;
    Core::Map<Red::TweakDBID, const RecordTypeInfo*> m_batchRecords;
};
}
