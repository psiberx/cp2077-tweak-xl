#include "StatService.hpp"
#include "App/Tweaks/TweakService.hpp"
#include "Core/Facades/Container.hpp"

namespace
{
constexpr auto BaseStatPrefix = "BaseStats.";
constexpr auto BaseStatPrefixLength = std::char_traits<char>::length(BaseStatPrefix);
constexpr auto BaseStatCount = static_cast<uint32_t>(Red::game::data::StatType::Count);
constexpr auto InvalidStat = static_cast<uint32_t>(Red::game::data::StatType::Invalid);

bool s_statTypesModified = false;
}

void App::StatService::OnBootstrap()
{
    HookAfter<Raw::StatsDataSystem::InitializeRecords>(&OnInitializeStats);
}

void App::StatService::OnInitializeStats(void* aSystem)
{
    auto statRecords = Raw::StatsDataSystem::StatRecords::Ptr(aSystem);
    auto statTypeEnum = Red::GetDescriptor<Red::game::data::StatType>();

    auto& tweakManager = Core::Resolve<TweakService>()->GetManager();
    auto& tweakChangelog = Core::Resolve<TweakService>()->GetChangelog();

    for (const auto& recordId : tweakChangelog.GetAffectedRecords())
    {
        const auto& recordName = tweakManager.GetName(recordId);
        if (recordName.starts_with(BaseStatPrefix))
        {
            auto enumName = tweakManager.GetFlat({recordId, ".enumName"}).As<Red::CString>().c_str();
            if (!statTypeEnum->HasOption(enumName))
            {
                if (recordName.substr(BaseStatPrefixLength) != enumName)
                {
                    LogError("{}: Enum name must match the record name.", recordName);
                    continue;
                }

                if (statRecords->size == BaseStatCount)
                {
                    // Add dummy entries for "Count" and "Invalid"
                    statRecords->EmplaceBack();
                    statRecords->EmplaceBack();
                }

                const auto enumValue = statRecords->size;

                statTypeEnum->AddOption(enumValue, enumName);
                statRecords->PushBack(recordId);

                if (!s_statTypesModified)
                {
                    s_statTypesModified = true;
                    Hook<Raw::StatsDataSystem::GetStatRange>(&OnGetStatRange);
                    Hook<Raw::StatsDataSystem::GetStatFlags>(&OnGetStatFlags);
                    Hook<Raw::StatsDataSystem::CheckStatFlag>(&OnCheckStatFlag);
                }

                {
                    const auto record = tweakManager.GetRecord(recordId);
                    const auto recordSize = record->GetType()->GetSize();
                    const auto enumPropAddr = reinterpret_cast<uintptr_t>(record.instance) + recordSize - 8;
                    *reinterpret_cast<uint32_t*>(enumPropAddr) = enumValue;
                }
            }
        }
    }
}

uint64_t* App::StatService::OnGetStatRange(void* aSystem, uint64_t* aRange, uint32_t aStat)
{
    if (aStat != InvalidStat)
    {
        auto& statParams = Raw::StatsDataSystem::StatParams::Ref(aSystem);
        auto& statLock = Raw::StatsDataSystem::StatLock::Ref(aSystem);

        std::shared_lock _(statLock);
        if (aStat < statParams.size)
        {
            *aRange = statParams[aStat].range;
            return aRange;
        }
    }

    *aRange = 0;
    return aRange;
}

uint32_t App::StatService::OnGetStatFlags(void* aSystem, uint32_t aStat)
{
    if (aStat != InvalidStat)
    {
        auto& statParams = Raw::StatsDataSystem::StatParams::Ref(aSystem);
        auto& statLock = Raw::StatsDataSystem::StatLock::Ref(aSystem);

        std::shared_lock _(statLock);
        if (aStat < statParams.size)
        {
            return statParams[aStat].flags;
        }
    }

    return 0;
}

bool App::StatService::OnCheckStatFlag(void* aSystem, uint32_t aStat, uint32_t aFlag)
{
    if (aStat != InvalidStat)
    {
        auto& statParams = Raw::StatsDataSystem::StatParams::Ref(aSystem);
        auto& statLock = Raw::StatsDataSystem::StatLock::Ref(aSystem);

        std::shared_lock _(statLock);
        if (aStat < statParams.size)
        {
            return statParams[aStat].flags & aFlag;
        }
    }

    return false;
}
