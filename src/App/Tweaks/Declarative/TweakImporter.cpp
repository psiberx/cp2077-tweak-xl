#include "TweakImporter.hpp"
#include "App/Tweaks/Batch/TweakChangeset.hpp"
#include "App/Tweaks/Declarative/Yaml/YamlReader.hpp"

App::TweakImporter::TweakImporter(Core::SharedPtr<Red::TweakDBManager> aManager)
    : m_manager(std::move(aManager))
{
}

void App::TweakImporter::ImportTweaks(const std::filesystem::path& aDir,
                                      const Core::SharedPtr<App::TweakChangelog>& aChangelog,
                                      bool aDryRun)
{
    std::error_code error;
    if (!std::filesystem::is_directory(aDir, error))
    {
        LogError(R"(Directory "{}" not found.)", aDir.string());
        return;
    }

    try
    {
        LogInfo("Scanning for tweaks...");

        TweakChangeset changeset;

        const auto tweakDirIt = std::filesystem::recursive_directory_iterator(
            aDir, std::filesystem::directory_options::follow_directory_symlink);

        for (const auto& entry : tweakDirIt)
        {
            if (entry.is_regular_file())
            {
                Read(changeset, entry.path(), aDir);
            }
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

void App::TweakImporter::ImportTweak(const std::filesystem::path& aPath,
                                     const Core::SharedPtr<App::TweakChangelog>& aChangelog,
                                     bool aDryRun)
{
    std::error_code error;
    if (!std::filesystem::is_regular_file(aPath, error))
    {
        LogError(R"(Tweak "{}" not found.)", aPath.string());
        return;
    }

    TweakChangeset changeset;

    if (Read(changeset, aPath, aPath.parent_path()))
    {
        if (!aDryRun)
        {
            Apply(changeset, aChangelog);
        }
    }
}

bool App::TweakImporter::Read(App::TweakChangeset& aChangeset, const std::filesystem::path& aPath,
                              const std::filesystem::path& aDir)
{
    Core::SharedPtr<ITweakReader> reader;

    {
        const auto ext = aPath.extension();

        if (ext == L".yaml" || ext == L".yml")
        {
            reader = Core::MakeShared<YamlReader>(m_manager);
        }
    }

    if (!reader)
    {
        return false;
    }

    try
    {
        LogInfo("Reading \"{}\"...", std::filesystem::relative(aPath, aDir).string());

        if (reader->Load(aPath))
        {
            reader->Read(aChangeset);
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

bool App::TweakImporter::Apply(App::TweakChangeset& aChangeset, const Core::SharedPtr<App::TweakChangelog>& aChangelog)
{
    if (aChangeset.IsEmpty())
    {
        LogInfo("Nothing to import.");
        return false;
    }

    LogInfo("Importing tweaks...");

    aChangeset.Commit(m_manager, aChangelog);

    LogInfo("Import completed.");

    return true;
}
