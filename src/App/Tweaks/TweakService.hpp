#pragma once

#include "Core/Foundation/Feature.hpp"

namespace App
{
class TweakService : public Core::Feature
{
public:
    TweakService(std::filesystem::path mTweakDir);

    void LoadAll();

    void ImportAll();
    void ImportDir(const std::filesystem::path& aPath);
    void ImportTweak(const std::filesystem::path& aPath);

    void ExecuteAll();
    void ExecuteTweak(RED4ext::CName aName);

private:
    std::filesystem::path m_tweakDir;
};
}
