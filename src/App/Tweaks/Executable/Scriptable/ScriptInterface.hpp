#pragma once

#include "Red/TweakDB/Reflection.hpp"

namespace App
{
class ScriptInterface : public Red::TweakDBInterface
{
public:
    static void SetReflection(Core::SharedPtr<Red::TweakDBReflection> aReflection);

private:
    using ScriptableHandle = Red::Handle<Red::IScriptable>;
    using ScriptableArray = Red::DynArray<ScriptableHandle>;
    using RecordHandle = Red::Handle<Red::TweakDBRecord>;
    using RecordArray = Red::DynArray<RecordHandle>;

    static void GetFlat(Red::IScriptable*, Red::CStackFrame* aFrame, Red::Variant* aRet, void*);
    static void GetRecord(Red::IScriptable*, Red::CStackFrame* aFrame, RecordHandle* aRet, void*);
    static void GetRecords(Red::IScriptable*, Red::CStackFrame* aFrame, RecordArray* aRet, void*);
    static void GetRecordCount(Red::IScriptable*, Red::CStackFrame* aFrame, uint32_t* aRet, void*);
    static void GetRecordByIndex(Red::IScriptable*, Red::CStackFrame* aFrame, RecordHandle* aRet, void*);

    static RecordArray* FetchRecords(Red::CName aTypeName);

    inline static Core::SharedPtr<Red::TweakDBReflection> s_reflection;

    RTTI_MEMBER_ACCESS(App::ScriptInterface);
};
}

RTTI_EXPAND_CLASS(Red::TweakDBInterface, App::ScriptInterface, {
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
