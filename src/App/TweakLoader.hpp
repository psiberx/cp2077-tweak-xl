#pragma once

#include "Core/Foundation/Feature.hpp"
#include "Core/Hooking/HookingAgent.hpp"
#include "Core/Logging/LoggingAgent.hpp"
#include "Engine/Scripting/RTTIClass.hpp"

namespace App
{
class TweakLoader
    : public Core::Feature
    , public Core::HookingAgent
    , public Core::LoggingAgent
    , public Engine::RTTIClass<TweakLoader>
{
public:
    static void Reload();
    static void ImportAll();
    static void ImportDir(RED4ext::CString& aPath);
    static void ImportTweak(RED4ext::CString& aPath);
    static void ExecuteAll();
    static void ExecuteTweak(RED4ext::CName aName);
    static RED4ext::CString Version();

private:
    void OnBootstrap() override;
    static void OnLoadTweakDB(RED4ext::TweakDB* aTweakDB, RED4ext::CString& aPath);

    friend Descriptor;
    static void OnRegister(Descriptor* aType);
    static void OnBuild(Descriptor* aType, RED4ext::CRTTISystem* aRtti);
};
}
