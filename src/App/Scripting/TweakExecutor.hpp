#pragma once

#include "stdafx.hpp"
#include "Core/Logging/LoggingAgent.hpp"
#include "TweakDB/Manager.hpp"

namespace App
{
class TweakExecutor : Core::LoggingAgent
{
public:
    explicit TweakExecutor(TweakDB::Manager& aManager);

    void ExecuteAll();
    void Execute(RED4ext::CName aTweakName);

private:
    bool Execute(RED4ext::CClass* aTweakClass);

    TweakDB::Manager& m_manager;
    RED4ext::CRTTISystem* m_rtti;
};
}
