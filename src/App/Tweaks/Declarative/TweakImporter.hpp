#pragma once

#include "App/Tweaks/Batch/TweakChangelog.hpp"
#include "App/Tweaks/Batch/TweakChangeset.hpp"
#include "App/Tweaks/Record/ScriptableRecordManager.hpp"
#include "App/Tweaks/TweakContext.hpp"
#include "Core/Logging/LoggingAgent.hpp"
#include "Red/TweakDB/Manager.hpp"
#include "TweakReader.hpp"

namespace App
{
class TweakImporter : Core::LoggingAgent
{
public:
    explicit TweakImporter(const Core::DeferredPtr<Red::TweakDBManager>& aManager,
                           const Core::DeferredPtr<Red::TweakDBReflection>& aReflection,
                           const Core::SharedPtr<ScriptableRecordManager>& aRecordManager,
                           const Core::SharedPtr<TweakContext>& aContext);

    void Load(const Core::Vector<std::filesystem::path>& aImportPaths);

    void ImportSchemas(const Core::SharedPtr<TweakChangelog>& aChangelog = nullptr, bool aDryRun = false) const;
    void ImportValues(const Core::SharedPtr<TweakChangelog>& aChangelog = nullptr, bool aDryRun = false) const;

private:
    Core::SharedPtr<ITweakReader> Load(const std::filesystem::path& aPath, const std::filesystem::path& aDir);

    bool Apply(const Core::SharedPtr<SchemaChangeset>& aChangeset,
               const Core::SharedPtr<TweakChangelog>& aChangelog) const;
    bool Apply(const Core::SharedPtr<TweakChangeset>& aChangeset,
               const Core::SharedPtr<TweakChangelog>& aChangelog) const;

    static bool IsFirstPriority(const std::filesystem::path& aPath);
    static bool IsLastPriority(const std::filesystem::path& aPath);

    Core::DeferredPtr<Red::TweakDBManager> m_manager;
    Core::DeferredPtr<Red::TweakDBReflection> m_reflection;
    Core::SharedPtr<ScriptableRecordManager> m_recordManager;
    Core::SharedPtr<TweakContext> m_context;

    Core::Vector<Core::SharedPtr<ITweakReader>> m_readers;
};
} // namespace App
