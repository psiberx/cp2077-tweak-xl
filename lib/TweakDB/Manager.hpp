#pragma once

#include "FlatPool.hpp"
#include "Reflection.hpp"
#include "Types.hpp"
#include "Raws.hpp"

namespace TweakDB
{
class Manager
{
public:
    Manager();
    explicit Manager(RED4ext::TweakDB* aTweakDb);

    Manager(const Manager&) = delete;
    Manager& operator=(const Manager&) = delete;

    RED4ext::CStackType GetFlat(RED4ext::TweakDBID aFlatId);
    RED4ext::Handle<RED4ext::gamedataTweakDBRecord> GetRecord(RED4ext::TweakDBID aRecordId);

    bool IsFlatExists(RED4ext::TweakDBID aFlatId);
    bool IsRecordExists(RED4ext::TweakDBID aRecordId);

    bool SetFlat(RED4ext::TweakDBID aFlatId, const RED4ext::CBaseRTTIType* aType, RED4ext::ScriptInstance aValue);
    bool SetFlat(RED4ext::TweakDBID aFlatId, RED4ext::CStackType aData);

    bool CreateRecord(RED4ext::TweakDBID aRecordId, const RED4ext::CClass* aType);
    bool CloneRecord(RED4ext::TweakDBID aRecordId, RED4ext::TweakDBID aSourceId);
    bool UpdateRecord(RED4ext::TweakDBID aRecordId);

    void RegisterName(RED4ext::TweakDBID aId, const std::string& aName);

    Reflection& GetReflection();

private:
    static uint32_t GetRecordTypeHash(RED4ext::Handle<RED4ext::IScriptable>* aRecord);

    inline static Raw::CreateRecord CreateRecordInternal;
    inline static Raw::CreateTweakDBID CreateTweakDBID;

    FlatPool m_flatPool;
    Reflection m_reflection;
    RED4ext::TweakDB* m_tweakDb;
};
}
