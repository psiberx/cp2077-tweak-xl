#pragma once

namespace Red::AddressLib
{
constexpr uint32_t Main = 240386859;

constexpr uint32_t StatsDataSystem_InitializeRecords = 1299190886;
constexpr uint32_t StatsDataSystem_InitializeParams = 3652194890;
constexpr uint32_t StatsDataSystem_GetStatRange = 1444748215;
constexpr uint32_t StatsDataSystem_GetStatFlags = 3123320294;
constexpr uint32_t StatsDataSystem_CheckStatFlag = 2954893634;

// Game 2.5 (3.0.88.46235) changed all three hashes (2.31 values: 3062572522, 3602585178, 3512345737).
constexpr uint32_t TweakDB_Init = 4053739225;        // RVA 0x219B850 on 2.5, no symbol row
constexpr uint32_t TweakDB_Load = 63182599;          // game::data::TweakDB::LoadOptimized, RVA 0x20F43D0 on 2.5
constexpr uint32_t TweakDB_TryLoad = 464332086;      // RVA 0x210A360 on 2.5, no symbol row
constexpr uint32_t TweakDB_CreateRecord = 838931066; // game::data::AddRecord

constexpr uint32_t TweakDBID_Derive = 326438016;
}
