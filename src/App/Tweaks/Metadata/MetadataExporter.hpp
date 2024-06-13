#pragma once

#include "App/Tweaks/Declarative/Red/RedReader.hpp"
#include "Red/TweakDB/Manager.hpp"
#include "Red/TweakDB/Source/Parser.hpp"

namespace App
{
class MetadataExporter
{
public:
    MetadataExporter(Core::SharedPtr<Red::TweakDBManager> aManager);

    bool LoadSource(const std::filesystem::path& aSourceDir);

    bool ExportInheritanceMap(const std::filesystem::path& aOutPath, bool aGeneratedComment = false);
    bool ExportExtraFlats(const std::filesystem::path& aOutPath, bool aGeneratedComment = false);

private:
    void ResolveGroups();

    static bool IsDebugGroup(const Red::TweakGroupPtr& aGroup);

    Core::SharedPtr<Red::TweakDBManager> m_manager;
    Core::SharedPtr<Red::TweakDBReflection> m_reflection;
    Core::Vector<Red::TweakSourcePtr> m_sources;
    Core::Map<std::string, Red::TweakGroupPtr> m_groups;
    Core::Map<std::string, std::string> m_records;
    bool m_resolved;
};
}
