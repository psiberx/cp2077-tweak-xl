#pragma once

#include "Red/TweakDB/Manager.hpp"

namespace App
{
class MetadataImporter
{
public:
    MetadataImporter(Core::SharedPtr<Red::TweakDBManager> aManager);

    bool ImportInheritanceMap(const std::filesystem::path& aPath);
    bool ImportExtraFlats(const std::filesystem::path& aPath);

private:
    Core::SharedPtr<Red::TweakDBManager> m_manager;
    Core::SharedPtr<Red::TweakDBReflection> m_reflection;
};
}
