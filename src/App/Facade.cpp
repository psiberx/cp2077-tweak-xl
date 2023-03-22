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

bool App::Facade::Require(Red::CString& aVersion)
{
    const auto requirement = semver::from_string_noexcept(aVersion.c_str());
    return requirement.has_value() ? Project::Version >= requirement.value() : false;
}

Red::CString App::Facade::GetVersion()
{
    return Project::Version.to_string().c_str();
}
