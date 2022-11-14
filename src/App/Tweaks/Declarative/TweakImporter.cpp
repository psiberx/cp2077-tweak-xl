#include "TweakImporter.hpp"
#include "App/Tweaks/Declarative/Yaml/YamlReader.hpp"

App::TweakImporter::TweakImporter(Core::SharedPtr<Red::TweakDB::Manager> aManager,
                                  std::filesystem::path aTweaksDir,
                                  Core::SharedPtr<App::TweakChangelog> aChangelog)
    : m_manager(std::move(aManager))
    , m_tweaksDir(std::move(aTweaksDir))
    , m_changelog(std::move(aChangelog))
{
}

void App::TweakImporter::ImportAll()
{
    ImportDir("");
}

void App::TweakImporter::ImportDir(const std::filesystem::path& aDir)
{
    if (!EnsureDirExists())
        return;

    try
    {
        LogInfo("Scanning for tweaks...");

        TweakChangeset changeset;

        const auto tweakDir = m_tweaksDir / aDir;
        const auto tweakDirIt = std::filesystem::recursive_directory_iterator(
            tweakDir, std::filesystem::directory_options::follow_directory_symlink);

        for (const auto& entry : tweakDirIt)
        {
            if (entry.is_regular_file())
            {
                ReadFile(changeset, entry.path());
            }
        }

        ApplyChangeset(changeset);
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

void App::TweakImporter::Import(const std::filesystem::path& aPath)
{
    if (!EnsureDirExists())
        return;

    if (!std::filesystem::exists(m_tweaksDir / aPath))
    {
        LogError(R"(Tweak "{}" not found.)", aPath.string());
        return;
    }

    TweakChangeset changeset;

    if (ReadFile(changeset, m_tweaksDir / aPath))
        ApplyChangeset(changeset);
}

bool App::TweakImporter::EnsureDirExists()
{
    std::error_code error;

    if (!std::filesystem::exists(m_tweaksDir, error))
    {
        if (!std::filesystem::create_directories(m_tweaksDir, error))
        {
            LogError("Cannot create tweaks directory [{}]: {}.",
                     m_tweaksDir.string(), error.message());

            return false;
        }
    }

    return true;
}

bool App::TweakImporter::ReadFile(App::TweakChangeset& aChangeset, const std::filesystem::path& aPath)
{
    Core::SharedPtr<ITweakReader> reader;

    {
        const auto ext = aPath.extension();

        if (ext == L".yaml" || ext == L".yml")
        {
            reader = Core::MakeShared<YamlReader>(*m_manager);
        }
    }

    if (!reader)
    {
        return false;
    }

    try
    {
        LogInfo("Reading \"{}\"...", std::filesystem::relative(aPath, m_tweaksDir).string());

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

bool App::TweakImporter::ApplyChangeset(App::TweakChangeset& aChangeset)
{
    if (aChangeset.IsEmpty())
    {
        LogInfo("Nothing to import.");
        return false;
    }

    LogInfo("Importing tweaks...");

    aChangeset.Commit(m_manager, m_changelog);

    LogInfo("Import completed.");

    return true;
}
