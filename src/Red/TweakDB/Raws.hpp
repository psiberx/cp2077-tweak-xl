#pragma once

#include "Alias.hpp"

namespace Raw
{
constexpr auto InitTweakDB = Core::RawFunc<
    /* addr = */ Red::AddressLib::TweakDB_Init,
    /* type = */ void (*)(void*, void*)>();

constexpr auto LoadTweakDB = Core::RawFunc<
    /* addr = */ Red::AddressLib::TweakDB_Load,
    /* type = */ void (*)(Red::TweakDB*, Red::CString&)>();

constexpr auto CreateRecord = Core::RawFunc<
    /* addr = */ Red::AddressLib::TweakDB_CreateRecord,
    /* type = */ void (*)(Red::TweakDB*, uint32_t, Red::TweakDBID)>();

constexpr auto CreateTweakDBID = Core::RawFunc<
    /* addr = */ Red::AddressLib::TweakDBID_Derive,
    /* type = */ void (*)(const Red::TweakDBID*, const Red::TweakDBID*, const char*)>();
}
