#pragma once

#include "Core/Facades/Runtime.hpp"

namespace App::Env
{
inline auto GameDir()
{
    return Core::Runtime::GetRootDir();
}

inline auto TweaksDir()
{
    return GameDir() / L"r6" / L"tweaks";
}

inline auto RedModSourcesDir()
{
    return GameDir() / L"tools" / L"redmod" / L"tweaks";
}

inline auto LegacyScriptsDir()
{
    return GameDir() / L"r6" / L"scripts" / L"TweakXL";
}

inline auto PluginDir()
{
    return Core::Runtime::GetModuleDir();
}

inline auto PluginScriptsDir()
{
    return PluginDir() / L"Scripts";
}

inline auto PluginDataDir()
{
    return PluginDir() / L"Data";
}

inline auto ExtraFlatsPath()
{
    return PluginDataDir() / L"ExtraFlats.dat";
}

inline auto InheritanceMapPath()
{
    return PluginDataDir() / L"InheritanceMap.dat";
}

inline const auto& GameVer()
{
    return Core::Runtime::GetHost()->GetProductVer();
}
}
