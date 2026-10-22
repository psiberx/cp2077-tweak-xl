#pragma once

#include "Alias.hpp"

namespace Raw
{
constexpr auto InitTweakDB = Core::RawFunc<
    /* addr = */ Red::AddressLib::TweakDB_Init,
    /* type = */ void (*)(void*, void*)>();

constexpr auto LoadTweakDB = Core::RawFunc<
    /* addr = */ Red::AddressLib::TweakDB_Load,
    /* type = */ void (*)(Red::TweakDB*, bool, Red::CString&)>(); // 2.5 added the bool (TweakDB_Init passes true)

constexpr auto TryLoadTweakDB = Core::RawFunc<
    /* addr = */ Red::AddressLib::TweakDB_TryLoad,
    /* type = */ bool (*)(void* a1, Red::TweakDB*, Red::CString&, void* a4, bool a5)>(); // 2.5 added a5 (stack)

constexpr auto CreateRecord = Core::RawFunc<
    /* addr = */ Red::AddressLib::TweakDB_CreateRecord,
    /* type = */ void (*)(Red::TweakDB*, uint32_t, Red::TweakDBID)>();

constexpr auto CreateTweakDBID = Core::RawFunc<
    /* addr = */ Red::AddressLib::TweakDBID_Derive,
    /* type = */ void (*)(const Red::TweakDBID*, const Red::TweakDBID*, const char*)>();
}
