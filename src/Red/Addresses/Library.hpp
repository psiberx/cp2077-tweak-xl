#pragma once

namespace Red::AddressLib
{
constexpr uint32_t Main = 240386859;

constexpr uint32_t StatsDataSystem_InitializeRecords = 1299190886;
constexpr uint32_t StatsDataSystem_InitializeParams = 3652194890;
constexpr uint32_t StatsDataSystem_GetStatRange = 1444748215;
constexpr uint32_t StatsDataSystem_GetStatFlags = 3123320294;
constexpr uint32_t StatsDataSystem_CheckStatFlag = 2954893634;

constexpr uint32_t TweakDB_Init = 3062572522;
constexpr uint32_t TweakDB_Load = 3602585178; // game::data::TweakDB::LoadOptimized
constexpr uint32_t TweakDB_TryLoad = 3512345737;
constexpr uint32_t TweakDB_CreateRecord = 838931066; // game::data::AddRecord

constexpr uint32_t TweakDBID_Derive = 326438016;
}
