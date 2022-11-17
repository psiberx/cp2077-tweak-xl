#pragma once

#include "Core/Logging/LoggingAgent.hpp"
#include "Red/TweakDB/Manager.hpp"

namespace App
{
class TweakExecutor : Core::LoggingAgent
{
public:
    explicit TweakExecutor(Core::SharedPtr<Red::TweakDBManager> aManager);

    void InitializeRuntime();

    void ExecuteTweaks();
    void ExecuteTweak(Red::CName aTweakName);

private:
    bool Execute(Red::CClass* aTweakClass);

    Red::CRTTISystem* m_rtti;
    Core::SharedPtr<Red::TweakDBManager> m_manager;
};
}
