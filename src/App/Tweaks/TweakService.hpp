#pragma once

#include "App/Tweaks/Batch/TweakChangelog.hpp"
#include "App/Tweaks/Declarative/TweakImporter.hpp"
#include "App/Tweaks/Executable/TweakExecutor.hpp"
#include "Core/Foundation/Feature.hpp"
#include "Core/Hooking/HookingAgent.hpp"
#include "Core/Logging/LoggingAgent.hpp"
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
    TweakService(std::filesystem::path aTweaksDir);

    void LoadTweaks();
    void ImportTweaks();
    void ExecuteTweaks();
    void ExecuteTweak(Red::CName aName);

protected:
    void OnBootstrap() override;
    void CreateTweaksDir();

    std::filesystem::path m_tweaksDir;
    Core::SharedPtr<Red::TweakDBReflection> m_reflection;
    Core::SharedPtr<Red::TweakDBManager> m_manager;
    Core::SharedPtr<App::TweakChangelog> m_changelog;
    Core::SharedPtr<App::TweakImporter> m_importer;
    Core::SharedPtr<App::TweakExecutor> m_executor;
};
}
