#include "Facade.hpp"
#include "App/Config.hpp"
#include "App/Project.hpp"
#include "App/Tweaks/TweakExecutor.hpp"
#include "App/Tweaks/TweakImporter.hpp"
#include "Engine/TweakDB/Manager.hpp"

void App::Facade::Reload()
{
    TweakDB::Manager manager;
    TweakImporter(manager, Config::GetTweaksDir()).ImportAll();
    TweakExecutor(manager).ExecuteAll();
}

void App::Facade::ImportAll()
{
    TweakDB::Manager manager;
    TweakImporter(manager, Config::GetTweaksDir()).ImportAll();
}

void App::Facade::ImportDir(RED4ext::CString& aPath)
{
    TweakDB::Manager manager;
    TweakImporter(manager, Config::GetTweaksDir()).ImportDir(aPath.c_str());
}

void App::Facade::ImportTweak(RED4ext::CString& aPath)
{
    TweakDB::Manager manager;
    TweakImporter(manager, Config::GetTweaksDir()).Import(aPath.c_str());
}

void App::Facade::ExecuteAll()
{
    TweakDB::Manager manager;
    TweakExecutor(manager).ExecuteAll();
}

void App::Facade::ExecuteTweak(RED4ext::CName aName)
{
    TweakDB::Manager manager;
    TweakExecutor(manager).Execute(aName);
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
