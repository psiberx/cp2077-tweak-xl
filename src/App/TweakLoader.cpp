#include "TweakLoader.hpp"
#include "Configuration.hpp"
#include "Import/TweakImporter.hpp"
#include "Scripting/TweakExecutor.hpp"
#include "TweakDB/Manager.hpp"
#include "TweakDB/Raws.hpp"
#include "Project.hpp"

void App::TweakLoader::OnBootstrap()
{
    RegisterRTTI();
    HookAfter<TweakDB::Raw::LoadTweakDB>(&OnLoadTweakDB);
}

void App::TweakLoader::OnLoadTweakDB(RED4ext::TweakDB* aTweakDB, RED4ext::CString& aPath)
{
    TweakDB::Manager manager(aTweakDB);
    TweakImporter(manager, Configuration::GetTweaksDir()).ImportAll();
    TweakExecutor(manager).ExecuteAll();
}

void App::TweakLoader::OnRegister(Descriptor* aType)
{
    aType->SetName("TweakXL");
    aType->SetFlags({ .isAbstract = true });
}

void App::TweakLoader::OnBuild(Descriptor* aType, RED4ext::CRTTISystem* aRtti)
{
    aType->AddFunction<&Reload>("Reload", { .isFinal = true });
    aType->AddFunction<&ImportAll>("ImportAll", { .isFinal = true });
    aType->AddFunction<&ImportDir>("ImportDir", { .isFinal = true });
    aType->AddFunction<&ImportTweak>("Import", { .isFinal = true });
    aType->AddFunction<&ExecuteAll>("ExecuteAll", { .isFinal = true });
    aType->AddFunction<&ExecuteTweak>("Execute", { .isFinal = true });
    aType->AddFunction<&Version>("Version", { .isFinal = true });
}

void App::TweakLoader::Reload()
{
    TweakDB::Manager manager;
    TweakImporter(manager, Configuration::GetTweaksDir()).ImportAll();
    TweakExecutor(manager).ExecuteAll();
}

void App::TweakLoader::ImportAll()
{
    TweakDB::Manager manager;
    TweakImporter(manager, Configuration::GetTweaksDir()).ImportAll();
}

void App::TweakLoader::ImportDir(RED4ext::CString& aPath)
{
    TweakDB::Manager manager;
    TweakImporter(manager, Configuration::GetTweaksDir()).ImportDir(aPath.c_str());
}

void App::TweakLoader::ImportTweak(RED4ext::CString& aPath)
{
    TweakDB::Manager manager;
    TweakImporter(manager, Configuration::GetTweaksDir()).Import(aPath.c_str());
}

void App::TweakLoader::ExecuteAll()
{
    TweakDB::Manager manager;
    TweakExecutor(manager).ExecuteAll();
}

void App::TweakLoader::ExecuteTweak(RED4ext::CName aName)
{
    TweakDB::Manager manager;
    TweakExecutor(manager).Execute(aName);
}

RED4ext::CString App::TweakLoader::Version()
{
    return Project::Version->to_string().c_str();
}
