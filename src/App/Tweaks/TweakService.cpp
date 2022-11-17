#include "TweakService.hpp"
#include "App/Tweaks/Declarative/TweakImporter.hpp"
#include "App/Tweaks/Executable/TweakExecutor.hpp"
#include "Red/TweakDB/Raws.hpp"

App::TweakService::TweakService(std::filesystem::path aTweaksDir)
    : m_tweaksDir(std::move(aTweaksDir))
{
}

void App::TweakService::OnBootstrap()
{
    HookAfter<Raw::LoadTweakDB>([&]() {
        m_reflection = Core::MakeShared<Red::TweakDBReflection>();
        m_manager = Core::MakeShared<Red::TweakDBManager>(m_reflection);

        m_changelog = Core::MakeShared<App::TweakChangelog>();
        m_importer = Core::MakeShared<App::TweakImporter>(m_manager, m_changelog, m_tweaksDir);
        m_executor = Core::MakeShared<App::TweakExecutor>(m_manager);

        LoadTweaks();
    });
}

void App::TweakService::LoadTweaks()
{
    if (m_manager)
    {
        m_importer->ImportTweaks();
        m_executor->ExecuteTweaks();
        m_changelog->CheckForIssues(m_manager);
        m_manager->Invalidate();
    }
}

void App::TweakService::ImportTweaks()
{
    if (m_manager)
    {
        m_importer->ImportTweaks();
        m_manager->Invalidate();
    }
}

void App::TweakService::ExecuteTweaks()
{
    if (m_manager)
    {
        m_executor->ExecuteTweaks();
        m_manager->Invalidate();
    }
}

void App::TweakService::ExecuteTweak(Red::CName aName)
{
    if (m_manager)
    {
        m_executor->ExecuteTweak(aName);
        m_manager->Invalidate();
    }
}
