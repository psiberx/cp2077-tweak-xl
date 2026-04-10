#pragma once

#include "Red/TweakDB/Manager.hpp"

namespace App
{
class MetadataImporter
{
public:
    MetadataImporter(Core::DeferredPtr<Red::TweakDBManager> aManager,
                     Core::DeferredPtr<Red::TweakDBReflection> aReflection);

    bool ImportInheritanceMap(const std::filesystem::path& aPath);
    bool ImportExtraFlats(const std::filesystem::path& aPath);

private:
    Core::DeferredPtr<Red::TweakDBManager> m_manager;
    Core::DeferredPtr<Red::TweakDBReflection> m_reflection;
};
} // namespace App
