#pragma once

#include "Alias.hpp"
#include "Core/Raw.hpp"
#include "Red/Addresses.hpp"

namespace Raw
{
constexpr auto LoadTweakDB = Core::RawFunc<
    /* addr = */ Red::Addresses::TweakDB_Load,
    /* type = */ void (*)(Red::TweakDB*, Red::CString&)>();

constexpr auto CreateRecord = Core::RawFunc<
    /* addr = */ Red::Addresses::TweakDB_CreateRecord,
    /* type = */ void (*)(Red::TweakDB*, uint32_t, Red::TweakDBID)>();

constexpr auto CreateTweakDBID = Core::RawFunc<
    /* addr = */ Red::Addresses::TweakDBID_Derive,
    /* type = */ void (*)(Red::TweakDBID*, const Red::TweakDBID*, const char*)>();

}
