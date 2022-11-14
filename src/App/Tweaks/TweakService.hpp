#pragma once

#include "App/Tweaks/Declarative/TweakChangelog.hpp"
#include "Core/Foundation/Feature.hpp"
#include "Core/Hooking/HookingAgent.hpp"

namespace App
{
class TweakService
    : public Core::Feature
    , public Core::HookingAgent
{
public:
    TweakService(std::filesystem::path mTweakDir);

    void LoadTweaks();
    void ImportTweaks();
    void ExecuteTweaks();
    void ExecuteTweak(RED4ext::CName aName);

protected:
    void OnBootstrap() override;

    Core::SharedPtr<Red::TweakDB::Manager> m_manager;
    Core::SharedPtr<App::TweakChangelog> m_changelog;
    // TODO: importer and executor
    std::filesystem::path m_tweakDir;
};
}
