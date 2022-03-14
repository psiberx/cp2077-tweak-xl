#pragma once

#include "Core/Foundation/Feature.hpp"
#include "Core/Hooking/HookingAgent.hpp"
#include "Core/Logging/LoggingAgent.hpp"

namespace App
{
class TweakLoader : public Core::Feature
    , public Core::HookingAgent
    , public Core::LoggingAgent
{
    void OnBootstrap() override;

    static void OnLoadTweakDB(RED4ext::TweakDB* aTweakDB, RED4ext::CString& aPath);
};
}
