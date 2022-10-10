#include "TweakExecutor.hpp"
#include "App/Tweaks/Executable/Scriptable/ScriptableTweak.hpp"
#include "App/Tweaks/Executable/Scriptable/ScriptedManager.hpp"

namespace
{
constexpr auto ApplyMethodName = "OnApply";
}

App::TweakExecutor::TweakExecutor(Red::TweakDB::Manager& aManager)
    : m_manager(aManager)
    , m_rtti(RED4ext::CRTTISystem::Get())
{
}

void App::TweakExecutor::ExecuteAll()
{
    try
    {
        RED4ext::DynArray<RED4ext::CClass*> tweakClasses;
        m_rtti->GetDerivedClasses(ScriptableTweak::GetRTTIType(), tweakClasses);

        if (tweakClasses.size == 0)
            return;

        LogInfo("Executing scriptable tweaks...");

        ScriptedManager scopedManager(m_manager);

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

void App::TweakExecutor::Execute(RED4ext::CName aTweakName)
{
    auto tweakClass = m_rtti->GetClass(aTweakName);

    if (!tweakClass)
    {
        LogError(R"(Tweak class "{}" not found.)", aTweakName.ToString());
        return;
    }

    if (!tweakClass->IsA(ScriptableTweak::GetRTTIType()))
    {
        LogError(R"(Tweak class "{}" must inherit from "{}".)",
                 aTweakName.ToString(), ScriptableTweak::GetRTTIName().ToString());
        return;
    }

    ScriptedManager scopedManager(m_manager);

    if (Execute(tweakClass))
        LogInfo("Execution completed.");
}

bool App::TweakExecutor::Execute(RED4ext::CClass* aTweakClass)
{
    try
    {
        auto applyCallback = aTweakClass->GetFunction(ApplyMethodName);

        if (!applyCallback || applyCallback->flags.hasUndefinedBody)
        {
            LogError(R"(Tweak class "{}" doesn't have "{}" method.)", ApplyMethodName);
            return false;
        }

        auto tweakHandle = ScriptableTweak::NewInstance(aTweakClass);

        if (!tweakHandle)
        {
            LogError(R"(Tweak instance "{}" cannot be constructed.)", aTweakClass->GetName().ToString());
            return false;
        }

        LogInfo(R"(Executing "{}"...)", aTweakClass->GetName().ToString());

        auto stack = RED4ext::CStack(tweakHandle.instance);

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
