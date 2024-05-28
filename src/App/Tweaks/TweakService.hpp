#pragma once

#include "App/Tweaks/Batch/TweakChangelog.hpp"
#include "App/Tweaks/Declarative/TweakImporter.hpp"
#include "App/Tweaks/Executable/TweakExecutor.hpp"
#include "App/Tweaks/TweakContext.hpp"
#include "Core/Foundation/Feature.hpp"
#include "Core/Hooking/HookingAgent.hpp"
#include "Core/Logging/LoggingAgent.hpp"
#include "Core/Runtime/HostImage.hpp"
#include "Red/TweakDB/Manager.hpp"
#include "Red/TweakDB/Reflection.hpp"

namespace App
{
class TweakService
    : public Core::Feature
    , public Core::HookingAgent
    , public Core::LoggingAgent
{
public:
    TweakService(std::filesystem::path aGameDir, std::filesystem::path aTweaksDir, const Core::SemvVer& aProductVer);

    bool RegisterTweak(std::filesystem::path aPath);
    bool RegisterDirectory(std::filesystem::path aPath);

    void LoadTweaks(bool aCheckForIssues);
    void ImportTweaks();
    void ExecuteTweaks();
    void ExecuteTweak(Red::CName aName);
    void CheckForIssues();

    Red::TweakDBManager& GetManager();
    Red::TweakDBReflection& GetReflection();
    App::TweakChangelog& GetChangelog();

protected:
    void OnBootstrap() override;
    void CreateTweaksDir();
    void ApplyPatches();

    std::filesystem::path m_gameDir;
    std::filesystem::path m_tweaksDir;
    const Core::SemvVer& m_productVer;
    Core::Vector<std::filesystem::path> m_importPaths;
    Core::SharedPtr<Red::TweakDBReflection> m_reflection;
    Core::SharedPtr<Red::TweakDBManager> m_manager;
    Core::SharedPtr<App::TweakChangelog> m_changelog;
    Core::SharedPtr<App::TweakImporter> m_importer;
    Core::SharedPtr<App::TweakExecutor> m_executor;
    Core::SharedPtr<App::TweakContext> m_context;
};
}
