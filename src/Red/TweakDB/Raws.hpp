#pragma once

#include "Core/Raw.hpp"
#include "Red/Addresses.hpp"

#include <RED4ext/Addresses.hpp>

namespace Raw
{
constexpr auto LoadTweakDB = Core::RawFunc<
    /* addr = */ Red::Addresses::TweakDB_Load,
    /* type = */ void (*)(RED4ext::TweakDB*, RED4ext::CString&)>();

constexpr auto CreateTweakDBID = Core::RawFunc<
    /* addr = */ Red::Addresses::TweakDBID_Derive,
    /* type = */ void (*)(RED4ext::TweakDBID*, const RED4ext::TweakDBID*, const char*)>();

constexpr auto CreateRecord = Core::RawFunc<
    /* addr = */ RED4ext::Addresses::TweakDB_CreateRecord,
    /* type = */ void (*)(RED4ext::TweakDB*, uint32_t, RED4ext::TweakDBID)>();
}
