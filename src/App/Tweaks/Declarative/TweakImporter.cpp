#include "TweakImporter.hpp"
#include "App/Tweaks/Batch/TweakChangeset.hpp"
#include "App/Tweaks/Declarative/Yaml/YamlReader.hpp"
#include "App/Tweaks/Declarative/Red/RedReader.hpp"

App::TweakImporter::TweakImporter(Core::SharedPtr<Red::TweakDBManager> aManager,
                                  Core::SharedPtr<App::TweakContext> aContext)
    : m_manager(std::move(aManager))
    , m_context(std::move(aContext))
{
}

void App::TweakImporter::ImportTweaks(const Core::Vector<std::filesystem::path>& aImportPaths,
                                      const Core::SharedPtr<App::TweakChangelog>& aChangelog,
                                      bool aDryRun)
{
    try
    {
        LogInfo("Scanning for tweaks...");

        auto changeset = Core::MakeShared<TweakChangeset>();

        for (const auto& importPath : aImportPaths)
        {
            std::error_code error;

            if (std::filesystem::is_directory(importPath, error))
            {
                const auto dirIt = std::filesystem::recursive_directory_iterator(
                    importPath, std::filesystem::directory_options::follow_directory_symlink);

                for (const auto& entry : dirIt)
                {
                    if (entry.is_regular_file())
                    {
                        Read(changeset, entry.path(), importPath);
                    }
                }
                continue;
            }

            if (std::filesystem::is_regular_file(importPath, error))
            {
                Read(changeset, importPath, importPath.parent_path());
                continue;
            }

            LogWarning("Can't import \"{}\".", importPath.string());
        }

        if (!aDryRun)
        {
            Apply(changeset, aChangelog);
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

bool App::TweakImporter::Read(const Core::SharedPtr<App::TweakChangeset>& aChangeset,
                              const std::filesystem::path& aPath,
                              const std::filesystem::path& aDir)
{
    Core::SharedPtr<ITweakReader> reader;

    {
        const auto ext = aPath.extension();

        if (ext == L".yaml" || ext == L".yml")
        {
            reader = Core::MakeShared<YamlReader>(m_manager, m_context);
        }
        else if (ext == L".tweak")
        {
            reader = Core::MakeShared<RedReader>(m_manager, m_context);
        }
    }

    if (!reader)
    {
        return false;
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
            reader->Read(*aChangeset);
        }
    }
    catch (const std::exception& ex)
    {
        LogError(ex.what());
        return false;
    }
    catch (...)
    {
        LogError("An unknown error occurred.");
        return false;
    }

    return true;
}

bool App::TweakImporter::Apply(const Core::SharedPtr<App::TweakChangeset>& aChangeset,
                               const Core::SharedPtr<App::TweakChangelog>& aChangelog)
{
    if (aChangeset->IsEmpty())
    {
        LogInfo("Nothing to import.");
        return false;
    }

    LogInfo("Importing tweaks...");

    aChangeset->Commit(m_manager, aChangelog);

    LogInfo("Import completed.");

    return true;
}
