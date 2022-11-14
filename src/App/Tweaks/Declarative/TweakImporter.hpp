#pragma once

#include "App/Tweaks/Declarative/TweakChangeset.hpp"
#include "App/Tweaks/Declarative/TweakChangelog.hpp"
#include "App/Tweaks/Declarative/TweakReader.hpp"
#include "Core/Logging/LoggingAgent.hpp"
#include "Red/TweakDB/Manager.hpp"
#include "Red/TweakDB/Reflection.hpp"

namespace App
{
class TweakImporter : Core::LoggingAgent
{
public:
    TweakImporter(Core::SharedPtr<Red::TweakDB::Manager> aManager, std::filesystem::path aTweaksDir,
                  Core::SharedPtr<App::TweakChangelog> aChangelog = nullptr);

    void ImportAll();
    void ImportDir(const std::filesystem::path& aPath);
    void Import(const std::filesystem::path& aPath);

private:
    bool EnsureDirExists();
    bool ReadFile(TweakChangeset& aChangeset, const std::filesystem::path& aFullPath);
    bool ApplyChangeset(TweakChangeset& aChangeset);

    Core::SharedPtr<Red::TweakDB::Manager> m_manager;
    Core::SharedPtr<App::TweakChangelog> m_changelog;
    std::filesystem::path m_tweaksDir;
};
}
