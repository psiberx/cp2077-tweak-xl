#pragma once

#include "Red/Rtti/Expansion.hpp"

namespace App
{
class ScriptedInterface : public Red::Rtti::Expansion<ScriptedInterface, RED4ext::game::data::TweakDBInterface>
{
private:
    using ScriptableHandle = RED4ext::Handle<RED4ext::IScriptable>;
    using ScriptableArray = RED4ext::DynArray<ScriptableHandle>;
    using RecordHandle = RED4ext::Handle<RED4ext::gamedataTweakDBRecord>;
    using RecordArray = RED4ext::DynArray<RecordHandle>;

    static void GetFlat(RED4ext::IScriptable*, RED4ext::CStackFrame* aFrame, RED4ext::Variant* aRet,
                        RED4ext::CBaseRTTIType* aRetType);
    static void GetRecord(RED4ext::IScriptable*, RED4ext::CStackFrame* aFrame, RecordHandle* aRet,
                          RED4ext::CRTTIHandleType* aRetType);
    static void GetRecords(RED4ext::IScriptable*, RED4ext::CStackFrame* aFrame, RecordArray* aRet,
                           RED4ext::CRTTIBaseArrayType* aRetType);
    static void GetRecordCount(RED4ext::IScriptable*, RED4ext::CStackFrame* aFrame, uint32_t* aRet, void*);
    static void GetRecordByIndex(RED4ext::IScriptable*, RED4ext::CStackFrame* aFrame, RecordHandle* aRet,
                                 RED4ext::CRTTIHandleType* aRetType);

    static ScriptableArray* FetchRecords(RED4ext::CName aTypeName);

    friend Descriptor;
    static void OnExpand(Descriptor* aType, RED4ext::CRTTISystem* aRtti);
};
}
