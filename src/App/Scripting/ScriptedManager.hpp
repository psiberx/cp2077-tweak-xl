#pragma once

#include "Engine/Scripting/RTTIClass.hpp"
#include "TweakDB/Manager.hpp"

namespace App
{
class ScriptedManager : public Engine::RTTIClass<ScriptedManager>
{
public:
    ScriptedManager() = default;
    explicit ScriptedManager(TweakDB::Manager& aManager);
    ~ScriptedManager() override;

private:
    static void SetFlat(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, bool* aRet, void*);
    static void CreateRecord(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, bool* aRet, void*);
    static void CloneRecord(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, bool* aRet, void*);
    static void UpdateRecord(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, bool* aRet, void*);
    static void RegisterName(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, bool* aRet, void*);

    friend Descriptor;
    static void OnRegister(Descriptor* aType);
    static void OnBuild(Descriptor* aType, RED4ext::CRTTISystem* aRtti);
};
}
