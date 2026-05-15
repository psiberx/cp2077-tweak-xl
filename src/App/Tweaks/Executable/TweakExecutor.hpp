#pragma once

#include "Core/Logging/LoggingAgent.hpp"
#include "Red/TweakDB/Manager.hpp"

namespace App
{
class TweakExecutor : Core::LoggingAgent
{
public:
    explicit TweakExecutor(Core::DeferredPtr<Red::TweakDBManager> aManager,
                           Core::DeferredPtr<Red::TweakDBReflection> aReflection);

    void InitializeRuntime();

    void ExecuteTweaks();
    void ExecuteTweak(Red::CName aTweakName);

private:
    bool Execute(Red::CClass* aTweakClass);

    Red::CRTTISystem* m_rtti;
    Core::DeferredPtr<Red::TweakDBManager> m_manager;
    Core::DeferredPtr<Red::TweakDBReflection> m_reflection;
};
} // namespace App
