#pragma once

#include "stdafx.hpp"
#include "TweakBatch.hpp"
#include "Core/Logging/LoggingAgent.hpp"
#include "TweakDB/Manager.hpp"
#include "TweakDB/Reflection.hpp"

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
    bool ReadTweakFile(TweakBatch& aBatch, const std::filesystem::path& aFullPath);
    bool ExecuteBatch(TweakBatch& aBatch);

    TweakDB::Manager& m_manager;
    TweakDB::Reflection& m_reflection;
    std::filesystem::path m_tweaksDir;
};
}
