#pragma once

#include "stdafx.hpp"
#include "Core/Facades/Runtime.hpp"

namespace App::Configuration
{
inline std::filesystem::path GetTweaksDir()
{
    return Core::Runtime::GetRootDir() / L"r6" / L"tweaks";
}
}
