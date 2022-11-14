#pragma once

#include "App/Tweaks/Declarative/TweakChangeset.hpp"
#include "App/Tweaks/Declarative/TweakReader.hpp"
#include "App/Tweaks/Declarative/Yaml/YamlConverter.hpp"
#include "Core/Logging/LoggingAgent.hpp"
#include "Red/TweakDB/Manager.hpp"
#include "Red/TweakDB/Reflection.hpp"

namespace App
{
class YamlReader
    : public ITweakReader
    , public Core::LoggingAgent
{
public:
    YamlReader(Red::TweakDB::Manager& aManager);
    ~YamlReader() override = default;

    bool Load(const std::filesystem::path& aPath) override;
    bool IsLoaded() const override;
    void Unload() override;
    void Read(TweakChangeset& aChangeset) override;

private:
    void HandleTopNode(TweakChangeset& aChangeset, const std::string& aName, const YAML::Node& aNode);
    void HandleFlatNode(TweakChangeset& aChangeset, const std::string& aName, const YAML::Node& aNode,
                        const RED4ext::CBaseRTTIType* aType = nullptr);
    void HandleRecordNode(TweakChangeset& aChangeset, const std::string& aPath, const std::string& aName,
                          const YAML::Node& aNode, const RED4ext::CClass* aType, RED4ext::TweakDBID aSourceId = 0);
    bool ResolveInlineNode(App::TweakChangeset& aChangeset, const std::string& aPath, const YAML::Node& aNode,
                           const RED4ext::CClass*& aForeignType, RED4ext::TweakDBID aSourceId);
    bool HandleRelativeChanges(TweakChangeset& aChangeset, const std::string& aPath, const std::string& aName,
                               const YAML::Node& aNode, const RED4ext::CBaseRTTIType* aElementType);
    static bool IsRelativeChange(const YAML::Node& aNode);

    const RED4ext::CBaseRTTIType* ResolveFlatType(const YAML::Node& aNode);
    const RED4ext::CBaseRTTIType* ResolveFlatType(RED4ext::CName aName);
    const RED4ext::CBaseRTTIType* ResolveFlatType(TweakChangeset& aChangeset, RED4ext::TweakDBID aFlatId);
    const RED4ext::CClass* ResolveRecordType(const YAML::Node& aNode);
    const RED4ext::CClass* ResolveRecordType(TweakChangeset& aChangeset, RED4ext::TweakDBID aRecordId);
    RED4ext::TweakDBID ResolveTweakDBID(const YAML::Node& aNode);

    void ConvertLegacyNodes();

    YAML::Node m_yaml;
    std::filesystem::path m_path;
    Red::TweakDB::Manager& m_manager;
    Red::TweakDB::Reflection& m_reflection;
    YamlConverter m_converter;
};
}
