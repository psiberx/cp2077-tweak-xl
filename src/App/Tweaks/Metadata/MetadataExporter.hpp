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

    void LoadSource(const std::filesystem::path& aSourceDir);
    void WriteInheritanceMap(const std::filesystem::path& aOutPath, bool aGeneratedComment = false);
    void WriteExtraFlats(const std::filesystem::path& aOutPath, bool aGeneratedComment = false);

private:
    void ResolveGroups();

    Core::SharedPtr<Red::TweakDBManager> m_manager;
    Core::Vector<Red::TweakSourcePtr> m_sources;
    Core::Map<std::string, Red::TweakGroupPtr> m_groups;
    Core::Map<std::string, std::string> m_records;
    bool m_resolved;
};
}
