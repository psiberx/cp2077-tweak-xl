#pragma once

#include "App/Tweaks/TweakChangeset.hpp"
#include "App/Tweaks/Yaml/YamlConverter.hpp"
#include "Core/Logging/LoggingAgent.hpp"
#include "Engine/TweakDB/Manager.hpp"
#include "Engine/TweakDB/Reflection.hpp"

namespace App
{
class YamlReader : public Core::LoggingAgent
{
public:
    YamlReader(TweakDB::Manager& aManager, TweakDB::Reflection& aReflection);

    bool Load(const std::filesystem::path& aPath);
    bool IsLoaded();
    void Unload();
    void Read(TweakChangeset& aChangeset);

private:
    void HandleTopNode(TweakChangeset& aChangeset, const std::string& aName, const YAML::Node& aNode);
    void HandleFlatNode(TweakChangeset& aChangeset, const std::string& aName, const YAML::Node& aNode,
                        const RED4ext::CBaseRTTIType* aType = nullptr);
    void HandleRecordNode(TweakChangeset& aChangeset, const std::string& aPath, const std::string& aName,
                          const YAML::Node& aNode, const RED4ext::CClass* aType, RED4ext::TweakDBID aSourceId = 0);
    void HandleInlineNode(App::TweakChangeset& aChangeset, const std::string& aPath, const std::string& aName,
                          const YAML::Node& aNode, const RED4ext::CClass* aType);
    bool HandleRelativeChanges(TweakChangeset& aChangeset, const std::string& aPath, const std::string& aName,
                               const YAML::Node& aNode, const RED4ext::CBaseRTTIType* aElementType);
    bool IsRelativeChange(const YAML::Node& aNode);

    const RED4ext::CBaseRTTIType* ResolveFlatType(const YAML::Node& aNode);
    const RED4ext::CBaseRTTIType* ResolveFlatType(RED4ext::CName aName);
    const RED4ext::CBaseRTTIType* ResolveFlatType(App::TweakChangeset& aChangeset, RED4ext::TweakDBID aFlatId);
    const RED4ext::CClass* ResolveRecordType(const YAML::Node& aNode);
    const RED4ext::CClass* ResolveRecordType(App::TweakChangeset& aChangeset, RED4ext::TweakDBID aRecordId);
    RED4ext::TweakDBID ResolveTweakDBID(const YAML::Node& aNode);

    void ConvertLegacyNodes();

    YAML::Node m_yaml;
    std::filesystem::path m_path;
    TweakDB::Manager& m_manager;
    TweakDB::Reflection& m_reflection;
    App::YamlConverter m_converter;
};
}
