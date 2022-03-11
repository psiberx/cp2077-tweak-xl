#include "TweakExecutor.hpp"
#include "ScriptableTweak.hpp"
#include "ScriptedManager.hpp"

namespace
{
constexpr auto ApplyCallbackName = "OnApply";
}

App::TweakExecutor::TweakExecutor(TweakDB::Manager& aManager)
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

        for (const auto* tweakClass : tweakClasses)
            Execute(tweakClass);

        LogInfo("Execution complete.");
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
        LogInfo("Execution complete.");
}

bool App::TweakExecutor::Execute(const RED4ext::CClass* aTweakClass)
{
    try
    {
        auto applyCallback = aTweakClass->GetFunction(ApplyCallbackName);

        if (!applyCallback)
        {
            LogError(R"(Tweak class "{}" doesn't have "{}" method.)", ApplyCallbackName);
            return false;
        }

        // Allocate memory...
        auto tweakInstance = aTweakClass->AllocInstance();

        if (!tweakInstance)
        {
            LogError(R"(Tweak instance "{}" cannot be allocated.)", aTweakClass->GetName().ToString());
            return false;
        }

        LogInfo(R"(Executing "{}"...)", aTweakClass->GetName().ToString());

        // Construct raw instance...
        aTweakClass->ConstructCls(tweakInstance);

        // Construct handle...
        auto scriptable = reinterpret_cast<RED4ext::ISerializable*>(tweakInstance);
        auto handle = RED4ext::Handle<RED4ext::ISerializable>(scriptable);

        // Assign handle to the instance...
        // This is required for instance to be recognized by scripting engine.
        // Without this reference scripted `this` will always be null.
        scriptable->ref = RED4ext::WeakHandle(handle);

        auto stack = RED4ext::CStack(tweakInstance);

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
