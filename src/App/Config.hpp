#pragma once

#include "Core/Facades/Runtime.hpp"

namespace App::Config
{
inline std::filesystem::path GetTweaksDir()
{
    return Core::Runtime::GetRootDir() / L"r6" / L"tweaks";
}
}
