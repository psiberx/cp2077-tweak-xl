#include "ScriptFacade.hpp"
#include "App/Configuration.hpp"
#include "App/Import/TweakImporter.hpp"
#include "App/Scripting/TweakExecutor.hpp"
#include "TweakDB/Manager.hpp"

void App::ScriptFacade::Reload()
{
    TweakDB::Manager manager;
    TweakImporter(manager, Configuration::GetTweaksDir()).ImportAll();
    TweakExecutor(manager).ExecuteAll();
}

void App::ScriptFacade::ImportAll()
{
    TweakDB::Manager manager;
    TweakImporter(manager, Configuration::GetTweaksDir()).ImportAll();
}

void App::ScriptFacade::ImportDir(RED4ext::CString& aPath)
{
    TweakDB::Manager manager;
    TweakImporter(manager, Configuration::GetTweaksDir()).ImportDir(aPath.c_str());
}

void App::ScriptFacade::ImportTweak(RED4ext::CString& aPath)
{
    TweakDB::Manager manager;
    TweakImporter(manager, Configuration::GetTweaksDir()).Import(aPath.c_str());
}

void App::ScriptFacade::ExecuteAll()
{
    TweakDB::Manager manager;
    TweakExecutor(manager).ExecuteAll();
}

void App::ScriptFacade::ExecuteTweak(RED4ext::CName aName)
{
    TweakDB::Manager manager;
    TweakExecutor(manager).Execute(aName);
}

void App::ScriptFacade::OnRegister(Descriptor* aType)
{
    aType->SetName("TweakXL");
    aType->SetFlags({ .isAbstract = true });
}

void App::ScriptFacade::OnBuild(Descriptor* aType, RED4ext::CRTTISystem* aRtti)
{
    aType->AddFunction<&Reload>("Reload", { .isFinal = true });
    aType->AddFunction<&ImportAll>("ImportAll", { .isFinal = true });
    aType->AddFunction<&ImportDir>("ImportDir", { .isFinal = true });
    aType->AddFunction<&ImportTweak>("Import", { .isFinal = true });
    aType->AddFunction<&ExecuteAll>("ExecuteAll", { .isFinal = true });
    aType->AddFunction<&ExecuteTweak>("Execute", { .isFinal = true });
}
