#include "Facade.hpp"
#include "App/Environment.hpp"
#include "App/Project.hpp"
#include "App/Tweaks/TweakService.hpp"
#include "Core/Facades/Container.hpp"

void App::Facade::Reload()
{
    Core::Resolve<TweakService>()->LoadAll();
}

void App::Facade::ImportAll()
{
    Core::Resolve<TweakService>()->ImportAll();
}

void App::Facade::ImportDir(RED4ext::CString& aPath)
{
    Core::Resolve<TweakService>()->ImportDir(aPath.c_str());
}

void App::Facade::ImportTweak(RED4ext::CString& aPath)
{
    Core::Resolve<TweakService>()->ImportTweak(aPath.c_str());
}

void App::Facade::ExecuteAll()
{
    Core::Resolve<TweakService>()->ExecuteAll();
}

void App::Facade::ExecuteTweak(RED4ext::CName aName)
{
    Core::Resolve<TweakService>()->ExecuteTweak(aName);
}

RED4ext::CString App::Facade::GetVersion()
{
    return Project::Version.to_string().c_str();
}

void App::Facade::OnRegister(Descriptor* aType)
{
    aType->SetName(App::Project::Name);
    aType->SetFlags({ .isAbstract = true });
}

void App::Facade::OnDescribe(Descriptor* aType, RED4ext::CRTTISystem*)
{
    aType->AddFunction<&Reload>("Reload", { .isFinal = true });
    aType->AddFunction<&ImportAll>("ImportAll", { .isFinal = true });
    aType->AddFunction<&ImportDir>("ImportDir", { .isFinal = true });
    aType->AddFunction<&ImportTweak>("Import", { .isFinal = true });
    aType->AddFunction<&ExecuteAll>("ExecuteAll", { .isFinal = true });
    aType->AddFunction<&ExecuteTweak>("Execute", { .isFinal = true });
    aType->AddFunction<&GetVersion>("Version", { .isFinal = true });
}
