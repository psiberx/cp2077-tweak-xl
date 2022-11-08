#pragma once

#include "App/Tweaks/Declarative/TweakChangeset.hpp"

namespace App
{
class ITweakReader
{
public:
    virtual ~ITweakReader() = default;
    virtual bool Load(const std::filesystem::path& aPath) = 0;
    virtual bool IsLoaded() const = 0;
    virtual void Unload() = 0;
    virtual void Read(TweakChangeset& aChangeset) = 0;
};
}
