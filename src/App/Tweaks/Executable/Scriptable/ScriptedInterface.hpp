#pragma once

#include "Red/TweakDB/Reflection.hpp"

namespace App
{
class ScriptedInterface : public Red::TweakDBInterface
{
public:
    static void SetReflection(Core::SharedPtr<Red::TweakDBReflection> aReflection);

private:
    using ScriptableHandle = Red::Handle<Red::IScriptable>;
    using ScriptableArray = Red::DynArray<ScriptableHandle>;
    using RecordHandle = Red::Handle<Red::TweakDBRecord>;
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

    inline static Core::SharedPtr<Red::TweakDBReflection> s_reflection;

    RTTI_MEMBER_ACCESS(App::ScriptedInterface);
};
}

RTTI_EXPAND_CLASS(Red::TweakDBInterface, App::ScriptedInterface, {
    // RTTI_METHOD(GetRecords, [](Red::CName type) -> App::ScriptedInterface::RecordArray {});
    // RTTI_METHOD(GetRecordCount);
    // RTTI_METHOD(GetRecordByIndex);
    // RTTI_METHOD(GetRecord);
    // RTTI_METHOD(GetFlat);

    {
        auto func = type->AddFunction(&Type::GetRecords, "GetRecords", { .isFinal = true });
        func->AddParam("CName", "type");
        func->SetReturnType("array:handle:gamedataTweakDBRecord");
    }
    {
        auto func = type->AddFunction(&Type::GetRecordCount, "GetRecordCount", { .isFinal = true });
        func->AddParam("CName", "type");
        func->SetReturnType("Int32");
    }
    {
        auto func = type->AddFunction(&Type::GetRecordByIndex, "GetRecordByIndex", { .isFinal = true });
        func->AddParam("CName", "type");
        func->AddParam("Int32", "index");
        func->SetReturnType("handle:gamedataTweakDBRecord");
    }
    {
        auto func = type->AddFunction(&Type::GetRecord, "GetRecord", { .isFinal = true });
        func->AddParam("TweakDBID", "path");
        func->SetReturnType("handle:gamedataTweakDBRecord");
    }
    {
        auto func = type->AddFunction(&Type::GetFlat, "GetFlat", { .isFinal = true });
        func->AddParam("TweakDBID", "path");
        func->SetReturnType("Variant");
    }
});
