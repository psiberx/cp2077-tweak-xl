#pragma once

#include "Core/Foundation/Application.hpp"
#include "Core/Hooking/HookingAgent.hpp"

namespace App
{
class Application
    : public Core::Application
    , public Core::HookingAgent
{
public:
    explicit Application(HMODULE aHandle, const RED4ext::Sdk* aSdk = nullptr);

protected:
    void OnBootstrap() override;
    static void OnTweakDBLoad(RED4ext::TweakDB* aTweakDB, RED4ext::CString& aPath);
};
}
