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
public:
    void Bootstrap() override;

private:
    static void OnLoadTweakDB(RED4ext::TweakDB* aTweakDB, RED4ext::CString& aPath);
};
}
