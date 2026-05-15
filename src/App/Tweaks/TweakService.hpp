#pragma once

#include "App/Tweaks/Batch/TweakChangelog.hpp"
#include "App/Tweaks/Declarative/TweakImporter.hpp"
#include "App/Tweaks/Executable/TweakExecutor.hpp"
#include "App/Tweaks/TweakContext.hpp"
#include "Core/Foundation/Feature.hpp"
#include "Core/Hooking/HookingAgent.hpp"
#include "Core/Logging/LoggingAgent.hpp"
#include "Core/Runtime/HostImage.hpp"
#include "Record/RedscriptTypesExporter.hpp"
#include "Record/ScriptableRecordManager.hpp"
#include "Red/TweakDB/Manager.hpp"
#include "Red/TweakDB/Raws.hpp"
#include "Red/TweakDB/Reflection.hpp"

namespace App
{
class TweakService
    : public Core::Feature
    , public Core::HookingAgent
    , public Core::LoggingAgent
{
public:
    using CreateRecordFunction = std::remove_cvref_t<decltype(Raw::CreateRecord)>::Callable;

    TweakService(const Core::SemvVer& aProductVer, std::filesystem::path aGameDir, std::filesystem::path aTweaksDir,
                 std::filesystem::path aPluginDir, std::filesystem::path aInheritanceMapPath,
                 std::filesystem::path aExtraFlatsPath, std::filesystem::path aSourcesDir,
                 std::filesystem::path aPluginScriptsDir);

    bool RegisterTweak(std::filesystem::path aPath);
    bool RegisterDirectory(std::filesystem::path aPath);

    void LoadTweaks(bool aCheckForIssues) const;

    void LoadFiles() const;
    void LoadSchemas() const;
    void LoadValues(bool aCheckForIssues, bool aExecute) const;

    void ImportTweaks() const;
    void ExecuteTweaks() const;
    void ExecuteTweak(Red::CName aName) const;
    void CheckForIssues() const;

    [[nodiscard]] bool ImportMetadata() const;
    void ExportMetadata();

    Core::DeferredPtr<Red::TweakDBManager> GetManager();
    Core::DeferredPtr<Red::TweakDBReflection> GetReflection();
    Core::DeferredPtr<TweakChangelog> GetChangelog();
    Core::SharedPtr<ScriptableRecordManager> GetRecordManager();
    Core::SharedPtr<ScriptablePropertyManager> GetPropertyHandler();

protected:
    void OnBootstrap() override;
    void CreateTweaksDir() const;
    void CreateScriptsDir() const;
    void EnsureRuntimeAccess() const;
    void ApplyPatches() const;
    void InsertPropertyDefaultValues() const;
    void OnValidateScripts(const Red::ScriptBundle* aBundle) const;

    std::filesystem::path m_gameDir;
    std::filesystem::path m_tweaksDir;
    std::filesystem::path m_pluginDir;
    std::filesystem::path m_sourcesDir;
    std::filesystem::path m_inheritanceMapPath;
    std::filesystem::path m_extraFlatsPath;
    std::filesystem::path m_pluginScriptsDir;
    const Core::SemvVer& m_productVer;
    Core::Vector<std::filesystem::path> m_importPaths;
    Core::DeferredPtr<Red::TweakDBReflection> m_reflection;
    Core::DeferredPtr<Red::TweakDBManager> m_manager;
    Core::SharedPtr<ScriptablePropertyManager> m_propertyHandler;
    Core::SharedPtr<ScriptableRecordManager> m_recordManager;
    Core::SharedPtr<TweakChangelog> m_changelog;
    Core::SharedPtr<TweakContext> m_context;
    Core::SharedPtr<TweakImporter> m_importer;
    Core::SharedPtr<TweakExecutor> m_executor;
    Core::SharedPtr<RedscriptTypesExporter> m_redscriptExporter;
};
} // namespace App
