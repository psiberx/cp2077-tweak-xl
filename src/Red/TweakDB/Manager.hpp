#pragma once

#include "Red/TweakDB/Alias.hpp"
#include "Red/TweakDB/Buffer.hpp"
#include "Red/TweakDB/Reflection.hpp"

namespace Red
{
class TweakDBManager
{
public:
    TweakDBManager();
    explicit TweakDBManager(Red::TweakDB* aTweakDb);
    explicit TweakDBManager(Core::SharedPtr<Red::TweakDBReflection> aReflection);

    TweakDBManager(const TweakDBManager&) = delete;
    TweakDBManager& operator=(const TweakDBManager&) = delete;

    Red::CStackType GetFlat(Red::TweakDBID aFlatId);
    Red::Handle<Red::TweakDBRecord> GetRecord(Red::TweakDBID aRecordId);

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

    void Invalidate();

    Red::TweakDB* GetTweakDB();
    Core::SharedPtr<Red::TweakDBReflection>& GetReflection();

private:
    void CreateBaseName(Red::TweakDBID aId, const std::string& aName);
    void CreateExtraNames(Red::TweakDBID aId, const std::string& aName);

    Red::TweakDB* m_tweakDb;
    Core::SharedPtr<Red::TweakDBBuffer> m_buffer;
    Core::SharedPtr<Red::TweakDBReflection> m_reflection;

    bool m_batchMode;
    Red::SortedUniqueArray<Red::TweakDBID> m_batchFlats;
    Core::Map<Red::TweakDBID, const Red::TweakDBRecordInfo*> m_batchRecords;
    Core::Map<Red::TweakDBID, const std::string> m_batchNames;
};
}
