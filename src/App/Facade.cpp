#include "Facade.hpp"
#include "App/Environment.hpp"
#include "App/Project.hpp"
#include "App/Tweaks/TweakService.hpp"
#include "Core/Facades/Container.hpp"
#include "Red/Log.hpp"

void App::Facade::Reload()
{
    Core::Resolve<TweakService>()->LoadTweaks();
}

void App::Facade::ImportAll()
{
    Core::Resolve<TweakService>()->ImportTweaks();
}

void App::Facade::ImportDir(Red::CString& aPath)
{
    Red::Log::Debug("[TweakXL] The method TweakXL.ImportDir() is no longer supported. Use TweakXL.Reload() instead.");
}

void App::Facade::ImportTweak(Red::CString& aPath)
{
    Red::Log::Debug("[TweakXL] The method TweakXL.ImportDir() is no longer supported. Use TweakXL.Reload() instead.");
}

void App::Facade::ExecuteAll()
{
    Core::Resolve<TweakService>()->ExecuteTweaks();
}

void App::Facade::ExecuteTweak(Red::CName aName)
{
    Core::Resolve<TweakService>()->ExecuteTweak(aName);
}

Red::CString App::Facade::GetVersion()
{
    return Project::Version.to_string().c_str();
}

void App::Facade::OnRegister(Descriptor* aType)
{
    aType->SetName(App::Project::Name);
    aType->SetFlags({ .isAbstract = true });
}

void App::Facade::OnDescribe(Descriptor* aType)
{
    aType->AddFunction<&Reload>("Reload", { .isFinal = true });
    aType->AddFunction<&ImportAll>("ImportAll", { .isFinal = true });
    aType->AddFunction<&ImportDir>("ImportDir", { .isFinal = true });
    aType->AddFunction<&ImportTweak>("Import", { .isFinal = true });
    aType->AddFunction<&ExecuteAll>("ExecuteAll", { .isFinal = true });
    aType->AddFunction<&ExecuteTweak>("Execute", { .isFinal = true });
    aType->AddFunction<&GetVersion>("Version", { .isFinal = true });
}
