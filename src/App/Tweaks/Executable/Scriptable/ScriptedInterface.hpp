#pragma once

#include "Red/Rtti/Expansion.hpp"
#include "Red/TweakDB/Reflection.hpp"

namespace App
{
class ScriptedInterface : public Red::Rtti::Expansion<ScriptedInterface, Red::game::data::TweakDBInterface>
{
public:
    static void SetReflection(Core::SharedPtr<Red::TweakDBReflection> aReflection);

private:
    using ScriptableHandle = Red::Handle<Red::IScriptable>;
    using ScriptableArray = Red::DynArray<ScriptableHandle>;
    using RecordHandle = Red::Handle<Red::gamedataTweakDBRecord>;
    using RecordArray = Red::DynArray<RecordHandle>;

    static void GetFlat(Red::IScriptable*, Red::CStackFrame* aFrame, Red::Variant* aRet,
                        Red::CBaseRTTIType* aRetType);
    static void GetRecord(Red::IScriptable*, Red::CStackFrame* aFrame, RecordHandle* aRet,
                          Red::CRTTIHandleType* aRetType);
    static void GetRecords(Red::IScriptable*, Red::CStackFrame* aFrame, RecordArray* aRet,
                           Red::CRTTIBaseArrayType* aRetType);
    static void GetRecordCount(Red::IScriptable*, Red::CStackFrame* aFrame, uint32_t* aRet, void*);
    static void GetRecordByIndex(Red::IScriptable*, Red::CStackFrame* aFrame, RecordHandle* aRet,
                                 Red::CRTTIHandleType* aRetType);

    static ScriptableArray* FetchRecords(Red::CName aTypeName);

    friend Descriptor;
    static void OnExpand(Descriptor* aType);

    inline static Core::SharedPtr<Red::TweakDBReflection> s_reflection;
};
}
