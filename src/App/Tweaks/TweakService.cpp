#include "TweakService.hpp"
#include "App/Tweaks/Declarative/TweakImporter.hpp"
#include "App/Tweaks/Executable/TweakExecutor.hpp"
#include "Red/TweakDB/Raws.hpp"

App::TweakService::TweakService(std::filesystem::path aGameDir, std::filesystem::path aTweaksDir)
    : m_gameDir(std::move(aGameDir))
    , m_tweaksDir(std::move(aTweaksDir))
{
    m_importPaths.push_back(m_tweaksDir);
}

void App::TweakService::OnBootstrap()
{
    CreateTweaksDir();

    HookAfter<Raw::LoadTweakDB>([&]() {
        m_reflection = Core::MakeShared<Red::TweakDBReflection>();
        m_manager = Core::MakeShared<Red::TweakDBManager>(m_reflection);

        m_changelog = Core::MakeShared<App::TweakChangelog>();
        m_importer = Core::MakeShared<App::TweakImporter>(m_manager);
        m_executor = Core::MakeShared<App::TweakExecutor>(m_manager);

        LoadTweaks();
    });
}

void App::TweakService::LoadTweaks()
{
    if (m_manager)
    {
        m_importer->ImportTweaks(m_importPaths, m_changelog);
        m_executor->ExecuteTweaks();
        m_changelog->CheckForIssues(m_manager);
    }
}

void App::TweakService::ImportTweaks()
{
    if (m_manager)
    {
        m_importer->ImportTweaks(m_importPaths, m_changelog);
    }
}

void App::TweakService::ExecuteTweaks()
{
    if (m_manager)
    {
        m_executor->ExecuteTweaks();
    }
}

void App::TweakService::ExecuteTweak(Red::CName aName)
{
    if (m_manager)
    {
        m_executor->ExecuteTweak(aName);
    }
}

void App::TweakService::CreateTweaksDir()
{
    std::error_code error;

    if (!std::filesystem::exists(m_tweaksDir, error))
    {
        if (!std::filesystem::create_directories(m_tweaksDir, error))
        {
            LogError("Cannot create tweaks directory \"{}\": {}.",
                     std::filesystem::relative(m_tweaksDir, m_gameDir).string(), error.message());
            return;
        }
    }
}

bool App::TweakService::RegisterTweak(std::filesystem::path aPath)
{
    std::error_code error;

    if (aPath.is_relative())
    {
        aPath = m_gameDir / aPath;
    }

    if (!std::filesystem::exists(aPath, error) || !std::filesystem::is_regular_file(aPath, error))
    {
        LogError("Can't register non-existing tweak \"{}\".",
                 std::filesystem::relative(aPath, m_gameDir).string());
        return false;
    }

    m_importPaths.emplace_back(std::move(aPath));
    return true;
}

bool App::TweakService::RegisterDirectory(std::filesystem::path aPath)
{
    std::error_code error;

    if (aPath.is_relative())
    {
        aPath = m_gameDir / aPath;
    }

    if (!std::filesystem::exists(aPath, error) || !std::filesystem::is_directory(aPath, error))
    {
        LogError("Can't register non-existing tweak directory \"{}\".",
                 std::filesystem::relative(aPath, m_gameDir).string());
        return false;
    }

    m_importPaths.emplace_back(std::move(aPath));
    return true;
}

Red::TweakDBManager& App::TweakService::GetManager()
{
    return *m_manager;
}

Red::TweakDBReflection& App::TweakService::GetReflection()
{
    return *m_reflection;
}

App::TweakChangelog& App::TweakService::GetChangelog()
{
    return *m_changelog;
}
