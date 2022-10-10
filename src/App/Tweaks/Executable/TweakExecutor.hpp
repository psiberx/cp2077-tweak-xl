#pragma once

#include "Core/Logging/LoggingAgent.hpp"
#include "Red/TweakDB/Manager.hpp"

namespace App
{
class TweakExecutor : Core::LoggingAgent
{
public:
    explicit TweakExecutor(Red::TweakDB::Manager& aManager);

    void ExecuteAll();
    void Execute(RED4ext::CName aTweakName);

private:
    bool Execute(RED4ext::CClass* aTweakClass);

    Red::TweakDB::Manager& m_manager;
    RED4ext::CRTTISystem* m_rtti;
};
}
