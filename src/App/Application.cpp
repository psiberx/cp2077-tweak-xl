#include "Application.hpp"
#include "App/Config.hpp"
#include "App/Facade.hpp"
#include "App/Tweaks/TweakExecutor.hpp"
#include "App/Tweaks/TweakImporter.hpp"
#include "App/Tweaks/Scriptable/ScriptableTweak.hpp"
#include "App/Tweaks/Scriptable/ScriptedInterface.hpp"
#include "App/Tweaks/Scriptable/ScriptedManager.hpp"
#include "Core/Runtime/RuntimeProvider.hpp"
#include "Engine/Scripting/RTTIProvider.hpp"
#include "Engine/TweakDB/Manager.hpp"
#include "Engine/TweakDB/Raws.hpp"
#include "Vendor/MinHook/MinHookProvider.hpp"
#include "Vendor/RED4ext/RED4extProvider.hpp"
#include "Vendor/Spdlog/SpdlogProvider.hpp"

App::Application::Application(HMODULE aHandle, const RED4ext::Sdk* aSdk)
{
    Register<Core::RuntimeProvider>(aHandle)->SetBaseImagePathDepth(2);

    Register<Vendor::MinHookProvider>();
    Register<Vendor::SpdlogProvider>();
    Register<Engine::RTTIProvider>();
}

void App::Application::OnBootstrap()
{
    HookAfter<TweakDB::Raw::LoadTweakDB>(&OnTweakDBLoad);
}

void App::Application::OnTweakDBLoad(RED4ext::TweakDB* aTweakDB, RED4ext::CString& aPath)
{
    TweakDB::Manager manager(aTweakDB);
    TweakImporter(manager, Config::GetTweaksDir()).ImportAll();
    TweakExecutor(manager).ExecuteAll();
}
