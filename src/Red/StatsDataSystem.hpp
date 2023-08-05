#pragma once

#include "Red/Addresses.hpp"

namespace Red
{
struct StatParams
{
#pragma pack(push, 1)
    union
    {
        uint64_t range;
        struct
        {
            float min;
            float max;
        };
    };
#pragma pack(pop)
    uint32_t flags;
};
}

namespace Raw::StatsDataSystem
{
using StatRecords = Core::OffsetPtr<0xD8, Red::DynArray<Red::TweakDBID>>;
using StatParams = Core::OffsetPtr<0xE8, Red::DynArray<Red::StatParams>>;
using StatLock = Core::OffsetPtr<0xFC, Red::SharedMutex>;

constexpr auto InitializeRecords = Core::RawFunc<
    /* addr = */ Red::Addresses::StatsDataSystem_InitializeRecords,
    /* type = */ void (*)(void* aSystem)>();

constexpr auto InitializeParams = Core::RawFunc<
    /* addr = */ Red::Addresses::StatsDataSystem_InitializeParams,
    /* type = */ void (*)(void* aSystem)>();

constexpr auto GetStatRange = Core::RawFunc<
    /* addr = */ Red::Addresses::StatsDataSystem_GetStatRange,
    /* type = */ uint64_t* (*)(void* aSystem, uint64_t*, uint32_t aStat)>();

constexpr auto GetStatFlags = Core::RawFunc<
    /* addr = */ Red::Addresses::StatsDataSystem_GetStatFlags,
    /* type = */ uint32_t (*)(void* aSystem, uint32_t aStat)>();

constexpr auto CheckStatFlag = Core::RawFunc<
    /* addr = */ Red::Addresses::StatsDataSystem_CheckStatFlag,
    /* type = */ bool (*)(void* aSystem, uint32_t aStat, uint32_t aFlag)>();
}
