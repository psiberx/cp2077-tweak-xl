#include "Application.hpp"
#include "App/Environment.hpp"
#include "App/Tweaks/TweakService.hpp"
#include "Core/Facades/Container.hpp"
#include "Core/Facades/Hook.hpp"
#include "Core/Foundation/RuntimeProvider.hpp"
#include "Red/Foundation/RttiProvider.hpp"
#include "Red/TweakDB/Raws.hpp"
#include "Vendor/MinHook/MinHookProvider.hpp"
#include "Vendor/RED4ext/RED4extProvider.hpp"
#include "Vendor/Spdlog/SpdlogProvider.hpp"

App::Application::Application(HMODULE aHandle, const RED4ext::Sdk*)
{
    Register<Core::RuntimeProvider>(aHandle)->SetBaseImagePathDepth(2);
    Register<Vendor::MinHookProvider>();
    Register<Vendor::SpdlogProvider>();
    Register<Red::RttiProvider>();
    Register<App::TweakService>(Env::TweaksDir());
}

void App::Application::OnBootstrap()
{
    HookAfter<Raw::LoadTweakDB>(+[]() {
        Resolve<App::TweakService>()->LoadAll();
    });
}
