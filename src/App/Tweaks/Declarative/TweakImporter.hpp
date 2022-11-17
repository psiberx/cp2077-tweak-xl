#pragma once

#include "App/Tweaks/Declarative/TweakChangelog.hpp"
#include "App/Tweaks/Declarative/TweakReader.hpp"
#include "Core/Logging/LoggingAgent.hpp"
#include "Red/TweakDB/Manager.hpp"

namespace App
{
class TweakImporter : Core::LoggingAgent
{
public:
    TweakImporter(Core::SharedPtr<Red::TweakDBManager> aManager, Core::SharedPtr<App::TweakChangelog> aChangelog,
                  std::filesystem::path aTweaksDir);

    void ImportTweaks();
    void ImportTweaks(const std::filesystem::path& aPath);
    void ImportTweak(const std::filesystem::path& aPath);

private:
    bool EnsureDirExists();
    bool ReadFile(TweakChangeset& aChangeset, const std::filesystem::path& aFullPath);
    bool ApplyChangeset(TweakChangeset& aChangeset);

    Core::SharedPtr<Red::TweakDBManager> m_manager;
    Core::SharedPtr<App::TweakChangelog> m_changelog;
    std::filesystem::path m_tweaksDir;
};
}
