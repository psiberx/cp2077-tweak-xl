#include "TweakImporter.hpp"

#include "App/Tweaks/Batch/TweakChangeset.hpp"
#include "App/Tweaks/Declarative/Red/RedReader.hpp"
#include "App/Tweaks/Declarative/Yaml/YamlReader.hpp"
#include "App/Tweaks/TweakService.hpp"

App::TweakImporter::TweakImporter(const Core::DeferredPtr<Red::TweakDBManager>& aManager,
                                  const Core::DeferredPtr<Red::TweakDBReflection>& aReflection,
                                  const Core::SharedPtr<ScriptableRecordManager>& aRecordManager,
                                  const Core::SharedPtr<TweakContext>& aContext)
    : m_manager(aManager)
    , m_reflection(aReflection)
    , m_recordManager(aRecordManager)
    , m_context(aContext)
{
}

void App::TweakImporter::Load(const Core::Vector<std::filesystem::path>& aImportPaths)
{
    LogInfo("Scanning for tweak files...");

    m_readers.clear();

    Core::Vector<std::pair<std::filesystem::path, std::filesystem::path>> firstPriorityPaths;
    Core::Vector<std::pair<std::filesystem::path, std::filesystem::path>> secondPriorityPaths;
    Core::Vector<std::pair<std::filesystem::path, std::filesystem::path>> lastPriorityPaths;
    std::error_code error;

    for (const auto& importPath : aImportPaths)
    {
        if (std::filesystem::is_directory(importPath, error))
        {
            const auto dirIt = std::filesystem::recursive_directory_iterator(
                importPath, std::filesystem::directory_options::follow_directory_symlink);
            for (const auto& entry : dirIt)
            {
                if (entry.is_regular_file())
                {
                    if (IsFirstPriority(entry.path()))
                    {
                        firstPriorityPaths.emplace_back(entry.path(), importPath);
                    }
                    else if (IsLastPriority(entry.path()))
                    {
                        lastPriorityPaths.emplace_back(entry.path(), importPath);
                    }
                    else
                    {
                        secondPriorityPaths.emplace_back(entry.path(), importPath);
                    }
                }
            }
            continue;
        }

        if (std::filesystem::is_regular_file(importPath, error))
        {
            if (IsFirstPriority(importPath))
            {
                firstPriorityPaths.emplace_back(importPath, importPath.parent_path());
            }
            else if (IsLastPriority(importPath))
            {
                lastPriorityPaths.emplace_back(importPath, importPath.parent_path());
            }
            else
            {
                secondPriorityPaths.emplace_back(importPath, importPath.parent_path());
            }
            continue;
        }

        LogWarning("Can't import \"{}\".", importPath.string());
    }

    for (const auto& [importPath, importDir] : firstPriorityPaths)
    {
        if (auto reader = Load(importPath, importDir))
        {
            m_readers.emplace_back(std::move(reader));
        }
    }

    for (const auto& [importPath, importDir] : secondPriorityPaths)
    {
        if (auto reader = Load(importPath, importDir))
        {
            m_readers.emplace_back(std::move(reader));
        }
    }

    for (const auto& [importPath, importDir] : lastPriorityPaths)
    {
        if (auto reader = Load(importPath, importDir))
        {
            m_readers.emplace_back(std::move(reader));
        }
    }
}

void App::TweakImporter::ImportSchemas(const Core::SharedPtr<TweakChangelog>& aChangelog, const bool aDryRun) const
{
    LogInfo("Scanning tweak files for schemas...");

    try
    {
        const auto changeset = Core::MakeShared<SchemaChangeset>();

        for (const auto& reader : m_readers)
        {
            if (reader->IsLoaded())
            {
                reader->ReadSchemas(*changeset);
            }
        }

        if (!aDryRun)
        {
            (void)Apply(changeset, aChangelog);
        }
    }
    catch (const std::exception& ex)
    {
        LogError(ex.what());
    }
    catch (...)
    {
        LogError("An unknown error occurred while trying to import schemas.");
    }
}

void App::TweakImporter::ImportValues(const Core::SharedPtr<TweakChangelog>& aChangelog, const bool aDryRun) const
{
    try
    {
        const auto changeset = Core::MakeShared<TweakChangeset>();

        for (const auto& reader : m_readers)
        {
            if (reader->IsLoaded())
            {
                reader->ReadValues(*changeset);
            }
        }

        if (!aDryRun)
        {
            (void)Apply(changeset, aChangelog);
        }
    }
    catch (const std::exception& ex)
    {
        LogError(ex.what());
    }
    catch (...)
    {
        LogError("An unknown error occurred while trying to import tweaks.");
    }
}

Core::SharedPtr<App::ITweakReader> App::TweakImporter::Load(const std::filesystem::path& aPath,
                                                            const std::filesystem::path& aDir)
{
    Core::SharedPtr<ITweakReader> reader;

    {
        if (const auto ext = aPath.extension(); ext == L".yaml" || ext == L".yml")
        {
            reader = Core::MakeShared<YamlReader>(m_manager, m_reflection, m_recordManager, m_context);
        }
        else if (ext == L".tweak")
        {
            reader = Core::MakeShared<RedReader>(m_manager, m_reflection, m_recordManager, m_context);
        }
    }

    if (!reader)
    {
        return nullptr;
    }

    try
    {
        std::error_code error;
        auto path = std::filesystem::relative(aPath, aDir, error);
        if (path.empty())
        {
            path = std::filesystem::absolute(aPath, error);
            path = std::filesystem::relative(path, aDir, error);
        }

        LogInfo("Reading \"{}\"...", path.string());

        if (reader->Load(aPath))
        {
            return reader;
        }
    }
    catch (const std::exception& ex)
    {
        LogError(ex.what());
    }
    catch (...)
    {
        LogError("An unknown error occurred.");
    }

    return nullptr;
}

bool App::TweakImporter::Apply(const Core::SharedPtr<SchemaChangeset>& aChangeset,
                               const Core::SharedPtr<TweakChangelog>& aChangelog) const
{
    if (aChangeset->IsEmpty() && aChangelog->IsEmpty())
    {
        LogInfo("No schemas to import.");
        return false;
    }

    LogInfo("Importing schemas...");

    aChangeset->Commit(m_recordManager);

    LogInfo("Schema import completed.");

    return true;
}

bool App::TweakImporter::Apply(const Core::SharedPtr<TweakChangeset>& aChangeset,
                               const Core::SharedPtr<TweakChangelog>& aChangelog) const
{
    if (aChangeset->IsEmpty() && aChangelog->IsEmpty())
    {
        LogInfo("Nothing to import.");
        return false;
    }

    LogInfo("Importing tweaks...");

    aChangeset->Commit(m_manager, aChangelog);

    LogInfo("Import completed.");

    return true;
}

bool App::TweakImporter::IsFirstPriority(const std::filesystem::path& aPath)
{
    static const std::string s_firstPriorityMarkers = "_#$!";
    return s_firstPriorityMarkers.find(aPath.filename().string().front()) != std::string::npos;
}

bool App::TweakImporter::IsLastPriority(const std::filesystem::path& aPath)
{
    static const std::string s_lastPriorityMarkers = "^";
    return s_lastPriorityMarkers.find(aPath.filename().string().front()) != std::string::npos;
}
