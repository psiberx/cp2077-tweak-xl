#include "TweakService.hpp"
#include "App/Tweaks/Declarative/TweakImporter.hpp"
#include "App/Tweaks/Executable/TweakExecutor.hpp"
#include "App/Tweaks/Metadata/MetadataExporter.hpp"
#include "App/Tweaks/Metadata/MetadataImporter.hpp"
#include "Core/Facades/Container.hpp"
#include "Record/CustomTweakDBRecord.hpp"
#include "Red/TweakDB/Raws.hpp"

App::TweakService::TweakService(const Core::SemvVer& aProductVer, std::filesystem::path aGameDir,
                                std::filesystem::path aTweaksDir, std::filesystem::path aInheritanceMapPath,
                                std::filesystem::path aExtraFlatsPath, std::filesystem::path aSourcesDir)
    : m_gameDir(std::move(aGameDir))
    , m_tweaksDir(std::move(aTweaksDir))
    , m_sourcesDir(std::move(aSourcesDir))
    , m_inheritanceMapPath(std::move(aInheritanceMapPath))
    , m_extraFlatsPath(std::move(aExtraFlatsPath))
    , m_productVer(aProductVer)
{
    m_importPaths.push_back(m_tweaksDir);
}

void App::TweakService::OnBootstrap()
{
    CreateTweaksDir();

    HookAfter<Raw::TryLoadTweakDB>([&](bool& aSuccess) {
        if (aSuccess)
        {
            m_reflection = Core::MakeShared<Red::TweakDBReflection>(Red::TweakDB::Get());
            m_manager = Core::MakeShared<Red::TweakDBManager>(m_reflection);
            m_context = Core::MakeShared<App::TweakContext>(m_productVer);
            m_importer = Core::MakeShared<App::TweakImporter>(m_manager, m_context);
            m_executor = Core::MakeShared<App::TweakExecutor>(m_manager);
            m_changelog = Core::MakeShared<App::TweakChangelog>();

#ifndef NDEBUG
            RegisterTestCustomRecord();
#endif

            if (ImportMetadata())
            {
                EnsureRuntimeAccess();
                ApplyPatches();
                LoadTweaks(false);
            }

#ifndef NDEBUG
            TestCustomRecord();
#endif
        }
    });

    HookAfter<Raw::InitTweakDB>([&]() {
        EnsureRuntimeAccess();
        CheckForIssues();
    });

    HookWrap<Raw::CreateRecord>([&](const CreateRecordFunction aOriginal, Red::TweakDB* aTweakDB,
                                    const uint32_t aTypeHash, const Red::TweakDBID aTweakDBID) {
        if (!m_manager || !m_manager->CreateCustomRecord(aTweakDB, aTweakDBID, aTypeHash))
            aOriginal(aTweakDB, aTypeHash, aTweakDBID);
    });
}

void App::TweakService::LoadTweaks(bool aCheckForIssues)
{
    if (m_manager)
    {
        m_importer->ImportTweaks(m_importPaths, m_changelog);
        m_executor->ExecuteTweaks();

        if (aCheckForIssues)
        {
            m_changelog->CheckForIssues(m_manager);
        }
    }
}

void App::TweakService::ImportTweaks()
{
    if (m_manager)
    {
        m_importer->ImportTweaks(m_importPaths, m_changelog);
    }
}

void App::TweakService::ExecuteTweaks()
{
    if (m_manager)
    {
        m_executor->ExecuteTweaks();
    }
}

void App::TweakService::ExecuteTweak(Red::CName aName)
{
    if (m_manager)
    {
        m_executor->ExecuteTweak(aName);
    }
}

void App::TweakService::EnsureRuntimeAccess()
{
    if (m_manager)
    {
        m_manager->GetTweakDB()->unk160 = 0;
    }
}

void App::TweakService::ApplyPatches()
{
    if (m_manager)
    {
        m_manager->CloneRecord("Vendors.IsPresent", "Vendors.Always_Present");
        m_manager->RegisterName("Vendors.IsPresent");
    }
}

void App::TweakService::CheckForIssues()
{
    if (m_manager && m_changelog)
    {
        m_changelog->CheckForIssues(m_manager);
    }
}

void App::TweakService::CreateTweaksDir()
{
    std::error_code error;

    if (!std::filesystem::exists(m_tweaksDir, error))
    {
        if (!std::filesystem::create_directories(m_tweaksDir, error))
        {
            LogWarning("Cannot create tweaks directory \"{}\": {}.",
                       std::filesystem::relative(m_tweaksDir, m_gameDir).string(), error.message());
        }
    }
}

bool App::TweakService::RegisterTweak(std::filesystem::path aPath)
{
    std::error_code error;

    if (aPath.is_relative())
    {
        aPath = m_gameDir / aPath;
    }

    if (!std::filesystem::exists(aPath, error) || !std::filesystem::is_regular_file(aPath, error))
    {
        LogError("Can't register non-existing tweak \"{}\".",
                 std::filesystem::relative(aPath, m_gameDir).string());
        return false;
    }

    m_importPaths.emplace_back(std::move(aPath));
    return true;
}

bool App::TweakService::RegisterDirectory(std::filesystem::path aPath)
{
    std::error_code error;

    if (aPath.is_relative())
    {
        aPath = m_gameDir / aPath;
    }

    if (!std::filesystem::exists(aPath, error) || !std::filesystem::is_directory(aPath, error))
    {
        LogError("Can't register non-existing tweak directory \"{}\".",
                 std::filesystem::relative(aPath, m_gameDir).string());
        return false;
    }

    m_importPaths.emplace_back(std::move(aPath));
    return true;
}

bool App::TweakService::ImportMetadata()
{
    MetadataImporter importer{m_manager};

    LogInfo("Loading inheritance metadata...");

    if (!importer.ImportInheritanceMap(m_inheritanceMapPath))
    {
        LogError("Can't load inheritance metadata from \"{}\".", m_inheritanceMapPath.string());
        return false;
    }

    LogInfo("Loading extra flats metadata...");

    if (!importer.ImportExtraFlats(m_extraFlatsPath))
    {
        LogError("Can't load extra flats metadata from \"{}\".", m_extraFlatsPath.string());
        return false;
    }

    return true;
}

void App::TweakService::ExportMetadata()
{
    MetadataExporter exporter{m_manager};
    exporter.LoadSource(m_sourcesDir);
    exporter.ExportInheritanceMap(m_inheritanceMapPath);
    exporter.ExportExtraFlats(m_extraFlatsPath);
    exporter.ExportInheritanceMap(m_inheritanceMapPath.replace_extension(".yaml"));
    exporter.ExportExtraFlats(m_extraFlatsPath.replace_extension(".yaml"));
}

Red::TweakDBManager& App::TweakService::GetManager()
{
    return *m_manager;
}

Red::TweakDBReflection& App::TweakService::GetReflection()
{
    return *m_reflection;
}

App::TweakChangelog& App::TweakService::GetChangelog()
{
    return *m_changelog;
}

void App::CustomRecordGetter(Red::IScriptable* aInstance, Red::CStackFrame* aStackFrame, void* aOut, int64_t)
{
    aStackFrame->code++;

    if (!aStackFrame->func || !aStackFrame->func->returnType || !aOut)
        return;

    const auto* func = aStackFrame->func;
    const auto* record = reinterpret_cast<App::CustomTweakDBRecord*>(aInstance);

    if (const auto* value =
            Core::Resolve<App::TweakService>()->GetManager().GetCustomRecordValue(record, func->shortName))
    {
        func->returnType->type->Assign(aOut, value);
    }
}

#ifndef NDEBUG

void App::TweakService::RegisterTestCustomRecord() const
{
    const auto recordInfo = m_reflection->CreateRecordInfo("TweakXLTest");
    recordInfo->isCustom = true;

    m_reflection->RegisterPropertyInfo(recordInfo, m_reflection->CreatePropertyInfo("foo", Red::ERTDBFlatType::CName));
    m_reflection->RegisterPropertyInfo(recordInfo, m_reflection->CreatePropertyInfo("bar", Red::ERTDBFlatType::CName));

    if (!m_manager->RegisterCustomRecord(recordInfo))
        LogError("Failed to register custom TweakDB record type {}.", recordInfo->name.ToString());

    if (!m_manager->DescribeCustomRecord(recordInfo, &CustomRecordGetter))
        LogError("Failed to describe custom TweakDB record type {}.", recordInfo->name.ToString());
}

void App::TweakService::TestCustomRecord()
{
    using recordType = Red::TypeLocator<Red::CName("gamedataTweakXLTest_Record")>;
    using cnameType = Red::TypeLocator<"CName">;

    static auto recordID = Red::TweakDBID{"test.tweakxl.custom"};
    static auto fooValue = Red::CNamePool::Add("test foo value");
    static auto barValue = Red::CNamePool::Add("test bar value");
    static auto fooAppendix = std::string_view(".foo");
    static auto barAppendix = std::string_view(".bar");

    assert(m_manager->CreateRecord(recordID, recordType::GetClass()));
    assert(m_manager->SetFlat(recordID + fooAppendix, cnameType::GetClass(), &fooValue));
    assert(m_manager->SetFlat(recordID + barAppendix, cnameType::GetClass(), &barValue));

    const auto record = reinterpret_cast<CustomTweakDBRecord*>(m_manager->GetTweakDB()->GetRecord(recordID).instance);

    assert(record);

    {
        auto* func = recordType::GetClass()->GetFunction("Foo");

        Red::CName result;
        Red::CStackType ret(func->returnType->type, &result);
        Red::CStack stack(record, nullptr, 0, &ret);

        assert(Red::CallFunction(func, stack));
        assert(result && strcmp(result.ToString(), "test foo value") == 0);
    }

    {
        auto* func = recordType::GetClass()->GetFunction("Bar");

        Red::CName result;
        Red::CStackType ret(func->returnType->type, &result);
        Red::CStack stack(record, nullptr, 0, &ret);

        assert(Red::CallFunction(func, stack));
        assert(result && strcmp(result.ToString(), "test bar value") == 0);
    }
}

#endif
