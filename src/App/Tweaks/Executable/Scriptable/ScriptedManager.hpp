#pragma once

#include "Red/Rtti/Class.hpp"
#include "Red/TweakDB/Manager.hpp"

namespace App
{
class ScriptedManager : public Red::Rtti::Class<ScriptedManager>
{
public:
    ScriptedManager() = default;
    explicit ScriptedManager(Red::TweakDB::Manager& aManager);
    ~ScriptedManager() override;

private:
    static void SetFlat(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, bool* aRet, void*);
    static void CreateRecord(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, bool* aRet, void*);
    static void CloneRecord(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, bool* aRet, void*);
    static void UpdateRecord(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, bool* aRet, void*);
    static void RegisterName(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, bool* aRet, void*);

    friend Descriptor;
    static void OnRegister(Descriptor* aType);
    static void OnDescribe(Descriptor* aType);
};
}
