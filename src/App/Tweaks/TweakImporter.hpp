#pragma once

#include "App/Tweaks/TweakChangeset.hpp"
#include "Core/Logging/LoggingAgent.hpp"
#include "Engine/TweakDB/Manager.hpp"
#include "Engine/TweakDB/Reflection.hpp"

namespace App
{
class TweakImporter : Core::LoggingAgent
{
public:
    TweakImporter(TweakDB::Manager& aManager, std::filesystem::path aTweaksDir);

    void ImportAll();
    void ImportDir(const std::filesystem::path& aPath);
    void Import(const std::filesystem::path& aPath);

private:
    bool EnsureDirExists();
    bool ReadTweakFile(TweakChangeset& aChangeset, const std::filesystem::path& aFullPath);
    bool ApplyChangeset(TweakChangeset& aChangeset);

    TweakDB::Manager& m_manager;
    TweakDB::Reflection& m_reflection;
    std::filesystem::path m_tweaksDir;
};
}
