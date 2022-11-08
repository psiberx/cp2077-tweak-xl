#pragma once

#include "App/Tweaks/Declarative/TweakChangeset.hpp"
#include "App/Tweaks/Declarative/TweakReader.hpp"
#include "Core/Logging/LoggingAgent.hpp"
#include "Red/TweakDB/Manager.hpp"
#include "Red/TweakDB/Reflection.hpp"

namespace App
{
class TweakImporter : Core::LoggingAgent
{
public:
    TweakImporter(Red::TweakDB::Manager& aManager, std::filesystem::path aTweaksDir);

    void ImportAll();
    void ImportDir(const std::filesystem::path& aPath);
    void Import(const std::filesystem::path& aPath);

private:
    bool EnsureDirExists();
    bool ReadFile(TweakChangeset& aChangeset, const std::filesystem::path& aFullPath);
    bool ApplyChangeset(TweakChangeset& aChangeset);

    Red::TweakDB::Manager& m_manager;
    Red::TweakDB::Reflection& m_reflection;
    std::filesystem::path m_tweaksDir;
};
}
