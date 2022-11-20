#pragma once

#include "Red/Rtti/Class.hpp"
#include "Red/TweakDB/Manager.hpp"

namespace App
{
class ScriptedManager : public Red::Rtti::Class<ScriptedManager>
{
public:
    static void SetManager(Core::SharedPtr<Red::TweakDBManager> aManager);

private:
    static void SetFlat(Red::IScriptable* aContext, Red::CStackFrame* aFrame, bool* aRet, void*);
    static void CreateRecord(Red::IScriptable* aContext, Red::CStackFrame* aFrame, bool* aRet, void*);
    static void CloneRecord(Red::IScriptable* aContext, Red::CStackFrame* aFrame, bool* aRet, void*);
    static void UpdateRecord(Red::IScriptable* aContext, Red::CStackFrame* aFrame, bool* aRet, void*);
    static void RegisterName(Red::IScriptable* aContext, Red::CStackFrame* aFrame, bool* aRet, void*);

    friend Descriptor;
    static void OnRegister(Descriptor* aType);
    static void OnDescribe(Descriptor* aType);

    inline static Core::SharedPtr<Red::TweakDBManager> s_manager;
    inline static Core::SharedPtr<Red::TweakDBReflection> s_reflection;
};
}
