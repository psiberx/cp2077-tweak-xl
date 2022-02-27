#pragma once

#include "Engine/Scripting/RTTIClass.hpp"

namespace App
{
class ScriptFacade : public Engine::RTTIClass<ScriptFacade>
{
public:
    static void Reload();
    static void ImportAll();
    static void ImportDir(RED4ext::CString& aPath);
    static void ImportTweak(RED4ext::CString& aPath);
    static void ExecuteAll();
    static void ExecuteTweak(RED4ext::CName aName);

private:
    friend Descriptor;
    static void OnRegister(Descriptor* aType);
    static void OnBuild(Descriptor* aType, RED4ext::CRTTISystem* aRtti);
};
}
