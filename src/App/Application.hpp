#pragma once

#include "Core/Foundation/Application.hpp"
#include "Core/Hooking/HookingAgent.hpp"
#include "Core/Logging/LoggingAgent.hpp"

namespace App
{
class Application
    : public Core::Application
    , public Core::LoggingAgent
    , public Core::HookingAgent
{
public:
    explicit Application(HMODULE aHandle, const RED4ext::Sdk* aSdk = nullptr);
};
}
