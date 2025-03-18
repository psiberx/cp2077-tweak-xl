#include "TweakExecutor.hpp"
#include "App/Tweaks/Executable/Scriptable/ScriptInterface.hpp"
#include "App/Tweaks/Executable/Scriptable/ScriptManager.hpp"
#include "App/Tweaks/Executable/Scriptable/ScriptableTweak.hpp"

namespace
{
constexpr auto ApplyMethodName = "OnApply";

static const Red::ClassLocator<App::ScriptableTweak> s_scriptableTweakType;
}

App::TweakExecutor::TweakExecutor(Core::SharedPtr<Red::TweakDBManager> aManager)
    : m_manager(std::move(aManager))
    , m_rtti(Red::CRTTISystem::Get())
{
    InitializeRuntime();
}

void App::TweakExecutor::InitializeRuntime()
{
    ScriptManager::SetManager(m_manager);
    ScriptInterface::SetReflection(m_manager->GetReflection());
}

void App::TweakExecutor::ExecuteTweaks()
{
    try
    {
        Red::DynArray<Red::CClass*> tweakClasses;
        m_rtti->GetClasses(s_scriptableTweakType, tweakClasses);

        if (tweakClasses.size == 0)
            return;

        LogInfo("Executing scriptable tweaks...");

        // ScriptedManager scopedManager(m_manager);

        for (auto* tweakClass : tweakClasses)
            Execute(tweakClass);

        LogInfo("Execution completed.");
    }
    catch (const std::exception& ex)
    {
        LogError(ex.what());
    }
    catch (...)
    {
        LogError("An unknown error occurred while trying to execute tweaks.");
    }
}

void App::TweakExecutor::ExecuteTweak(Red::CName aTweakName)
{
    auto tweakClass = m_rtti->GetClass(aTweakName);

    if (!tweakClass)
    {
        LogError(R"(Tweak class "{}" not found.)", aTweakName.ToString());
        return;
    }

    if (!tweakClass->IsA(s_scriptableTweakType))
    {
        LogError(R"(Tweak class "{}" must inherit from "{}".)",
                 aTweakName.ToString(), s_scriptableTweakType->GetName().ToString());
        return;
    }

    if (tweakClass->flags.isAbstract)
    {
        LogError(R"(Tweak class "{}" is abstract and cannot be executed.)", aTweakName.ToString());
        return;
    }

    // ScriptedManager scopedManager(m_manager);

    if (Execute(tweakClass))
        LogInfo("Execution completed.");
}

bool App::TweakExecutor::Execute(Red::CClass* aTweakClass)
{
    try
    {
        if (aTweakClass->flags.isAbstract)
            return false;

        auto applyCallback = aTweakClass->GetFunction(ApplyMethodName);

        if (!applyCallback || applyCallback->flags.hasUndefinedBody)
        {
            LogError(R"(Tweak class "{}" doesn't have "{}" method.)",
                     aTweakClass->GetName().ToString(), ApplyMethodName);
            return false;
        }

        auto tweakHandle = Red::Handle(reinterpret_cast<ScriptableTweak*>(aTweakClass->CreateInstance()));

        if (!tweakHandle)
        {
            LogError(R"(Tweak instance "{}" cannot be constructed.)",
                     aTweakClass->GetName().ToString());
            return false;
        }

        LogInfo(R"(Executing "{}"...)", aTweakClass->GetName().ToString());

        auto stack = Red::CStack(tweakHandle.instance);

        applyCallback->Execute(&stack);
    }
    catch (const std::exception& ex)
    {
        LogError(ex.what());
        return false;
    }
    catch (...)
    {
        LogError("An unknown error occurred.");
        return false;
    }

    return true;
}
