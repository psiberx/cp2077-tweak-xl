#pragma once

#include "Core/Runtime/HostImage.hpp"

namespace App
{
class TweakContext
{
public:
    TweakContext(const Core::SemvVer& aProductVer)
        : m_gameVersion(static_cast<uint8_t>(aProductVer.major),
                        static_cast<uint8_t>(aProductVer.minor),
                        static_cast<uint8_t>(aProductVer.patch))
        , m_isEpisodeOne(false)
    {
        Red::CallGlobal("IsEP1", m_isEpisodeOne);
    }

    [[nodiscard]] inline bool CheckGameVersion(const std::string& aCondition) const
    {
        return semver::range::satisfies(m_gameVersion, aCondition);
    }

    [[nodiscard]] inline bool CheckInstalledDLC(const std::string& aCondition) const
    {
        if (aCondition == "EP1") {
            return m_isEpisodeOne;
        }

        return aCondition.empty();
    }

private:
    semver::version m_gameVersion;
    bool m_isEpisodeOne;
};
}
