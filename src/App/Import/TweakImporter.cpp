#include "TweakImporter.hpp"
#include "Yaml/YamlReader.hpp"

App::TweakImporter::TweakImporter(TweakDB::Manager& aManager, std::filesystem::path aTweaksDir)
    : m_manager(aManager)
    , m_reflection(aManager.GetReflection())
    , m_tweaksDir(std::move(aTweaksDir))
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

        TweakBatch batch(m_manager);

        auto tweakDirIt = std::filesystem::recursive_directory_iterator(
            m_tweaksDir / aDir, std::filesystem::directory_options::follow_directory_symlink);

        for (const auto& entry : tweakDirIt)
        {
            const auto ext = entry.path().extension();

            if (entry.is_regular_file() && (ext == L".yml" || ext == L".yaml"))
            {
                ReadTweakFile(batch, entry.path());
            }
        }

        ExecuteBatch(batch);
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

    TweakBatch batch(m_manager);

    if (ReadTweakFile(batch, m_tweaksDir / aPath))
        ExecuteBatch(batch);
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

bool App::TweakImporter::ReadTweakFile(App::TweakBatch& aBatch, const std::filesystem::path& aFullPath)
{
    try
    {
        LogInfo("Reading \"{}\"...", std::filesystem::relative(aFullPath, m_tweaksDir).string());

        YamlReader reader(m_manager, m_reflection);

        if (reader.Load(aFullPath))
            reader.Read(aBatch);
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

bool App::TweakImporter::ExecuteBatch(App::TweakBatch& aBatch)
{
    if (aBatch.IsEmpty())
    {
        LogInfo("Nothing to import.");
        return false;
    }

    LogInfo("Importing tweaks...");

    aBatch.Dispatch();

    LogInfo("Import complete.");

    return true;
}
