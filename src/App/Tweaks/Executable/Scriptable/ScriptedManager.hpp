#pragma once

#include "Red/TweakDB/Manager.hpp"

namespace App
{
class ScriptedManager : public Red::IScriptable
{
public:
    static void SetManager(Core::SharedPtr<Red::TweakDBManager> aManager);

private:
    static void SetFlat(Red::IScriptable* aContext, Red::CStackFrame* aFrame, bool* aRet, void*);
    static void CreateRecord(Red::IScriptable* aContext, Red::CStackFrame* aFrame, bool* aRet, void*);
    static void CloneRecord(Red::IScriptable* aContext, Red::CStackFrame* aFrame, bool* aRet, void*);
    static void UpdateRecord(Red::IScriptable* aContext, Red::CStackFrame* aFrame, bool* aRet, void*);
    static void RegisterName(Red::IScriptable* aContext, Red::CStackFrame* aFrame, bool* aRet, void*);

    inline static Core::SharedPtr<Red::TweakDBManager> s_manager;
    inline static Core::SharedPtr<Red::TweakDBReflection> s_reflection;

    RTTI_IMPL_TYPEINFO(App::ScriptedManager);
    RTTI_DECLARE_FRIENDS(App::ScriptedManager);
};
}

RTTI_DEFINE_CLASS(App::ScriptedManager, "TweakDBManager", {
    type->MarkAbstract();
    {
        auto func = type->AddFunction(&Type::SetFlat, "SetFlat", { .isFinal = true });
        func->AddParam("TweakDBID", "path");
        func->AddParam("Variant", "value");
        func->SetReturnType("Bool");
    }
    {
        auto func = type->AddFunction(&Type::CreateRecord, "CreateRecord", { .isFinal = true });
        func->AddParam("TweakDBID", "path");
        func->AddParam("CName", "type");
        func->SetReturnType("Bool");
    }
    {
        auto func = type->AddFunction(&Type::CloneRecord, "CloneRecord", { .isFinal = true });
        func->AddParam("TweakDBID", "path");
        func->AddParam("TweakDBID", "base");
        func->SetReturnType("Bool");
    }
    {
        auto func = type->AddFunction(&Type::UpdateRecord, "UpdateRecord", { .isFinal = true });
        func->AddParam("TweakDBID", "path");
        func->SetReturnType("Bool");
    }
    {
        auto func = type->AddFunction(&Type::RegisterName, "RegisterName", { .isFinal = true });
        func->AddParam("TweakDBID", "path");
        func->SetReturnType("Bool");
    }
})
