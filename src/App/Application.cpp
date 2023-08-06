#include "Application.hpp"
#include "App/Environment.hpp"
#include "App/Project.hpp"
#include "App/Stats/StatService.hpp"
#include "App/Tweaks/TweakService.hpp"
#include "Core/Foundation/RuntimeProvider.hpp"
#include "Support/MinHook/MinHookProvider.hpp"
#include "Support/RedLib/RedLibProvider.hpp"
#include "Support/Spdlog/SpdlogProvider.hpp"

App::Application::Application(HMODULE aHandle, const RED4ext::Sdk*)
{
    Register<Core::RuntimeProvider>(aHandle)->SetBaseImagePathDepth(2);

    Register<Support::MinHookProvider>();
    Register<Support::SpdlogProvider>();
    Register<Support::RedLibProvider>();

    Register<App::TweakService>(Env::GameDir(), Env::TweaksDir());
    Register<App::StatService>();
}

void App::Application::OnStarting()
{
    LogInfo("{} {} is starting...", Project::Name, Project::Version.to_string());
}
