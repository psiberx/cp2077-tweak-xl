#pragma once

#include "App/Tweaks/Batch/TweakChangelog.hpp"
#include "App/Tweaks/Batch/TweakChangeset.hpp"
#include "App/Tweaks/Declarative/TweakReader.hpp"
#include "Core/Logging/LoggingAgent.hpp"
#include "Red/TweakDB/Manager.hpp"

namespace App
{
class TweakImporter : Core::LoggingAgent
{
public:
    TweakImporter(Core::SharedPtr<Red::TweakDBManager> aManager);

    void ImportTweaks(const Core::Vector<std::filesystem::path>& aImportPaths,
                      const Core::SharedPtr<App::TweakChangelog>& aChangelog = nullptr,
                      bool aDryRun = false);

private:
    bool Read(App::TweakChangeset& aChangeset, const std::filesystem::path& aPath, const std::filesystem::path& aDir);
    bool Apply(TweakChangeset& aChangeset, const Core::SharedPtr<App::TweakChangelog>& aChangelog);

    Core::SharedPtr<Red::TweakDBManager> m_manager;
};
}
