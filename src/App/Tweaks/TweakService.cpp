#include "TweakService.hpp"
#include "App/Tweaks/Declarative/TweakImporter.hpp"
#include "App/Tweaks/Executable/TweakExecutor.hpp"
#include "Red/TweakDB/Manager.hpp"
#include "Red/TweakDB/Raws.hpp"

App::TweakService::TweakService(std::filesystem::path mTweakDir)
    : m_tweakDir(std::move(mTweakDir))
{
}

void App::TweakService::OnBootstrap()
{
    HookAfter<Raw::LoadTweakDB>([&]() {
        m_manager = Core::MakeShared<Red::TweakDB::Manager>();
        m_changelog = Core::MakeShared<App::TweakChangelog>();

        LoadTweaks();
    });
}

void App::TweakService::LoadTweaks()
{
    if (m_manager)
    {
        TweakImporter(m_manager, m_tweakDir, m_changelog).ImportAll();
        TweakExecutor(*m_manager).ExecuteAll();

        m_changelog->CheckForIssues(m_manager);
        m_manager->InvalidateFlatPool();
    }
}

void App::TweakService::ImportTweaks()
{
    if (m_manager)
    {
        TweakImporter(m_manager, m_tweakDir, m_changelog).ImportAll();

        m_manager->InvalidateFlatPool();
    }
}

void App::TweakService::ExecuteTweaks()
{
    if (m_manager)
    {
        TweakExecutor(*m_manager).ExecuteAll();

        m_manager->InvalidateFlatPool();
    }
}

void App::TweakService::ExecuteTweak(RED4ext::CName aName)
{
    if (m_manager)
    {
        TweakExecutor(*m_manager).Execute(aName);

        m_manager->InvalidateFlatPool();
    }
}
