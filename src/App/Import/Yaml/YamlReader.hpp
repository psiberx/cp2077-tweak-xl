#pragma once

#include "stdafx.hpp"
#include "App/Import/TweakBatch.hpp"
#include "App/Import/Yaml/YamlConverter.hpp"
#include "Core/Logging/LoggingAgent.hpp"
#include "TweakDB/Manager.hpp"
#include "TweakDB/Reflection.hpp"

namespace App
{
class YamlReader : public Core::LoggingAgent
{
public:
    YamlReader(TweakDB::Manager& aManager, TweakDB::Reflection& aReflection);

    bool Load(const std::filesystem::path& aPath);
    bool IsLoaded();
    void Unload();
    void Read(TweakBatch& aBatch);

private:
    void HandleTopNode(TweakBatch& aBatch, const std::string& aName, const YAML::Node& aNode);
    void HandleFlatNode(TweakBatch& aBatch, const std::string& aName, const YAML::Node& aNode,
                        const RED4ext::CBaseRTTIType* aType = nullptr);
    void HandleRecordNode(TweakBatch& aBatch, const std::string& aPath, const std::string& aName,
                          const YAML::Node& aNode, const RED4ext::CClass* aType, RED4ext::TweakDBID aSourceId = 0);
    void HandleInlineNode(App::TweakBatch& aBatch, const std::string& aPath, const std::string& aName,
                          const YAML::Node& aNode, const RED4ext::CClass* aType);
    bool HandleRelativeChanges(TweakBatch& aBatch, const std::string& aPath, const std::string& aName,
                               const YAML::Node& aNode, const RED4ext::CBaseRTTIType* aElementType);
    bool IsRelativeChange(const YAML::Node& aNode);

    const RED4ext::CBaseRTTIType* ResolveFlatType(const YAML::Node& aNode);
    const RED4ext::CBaseRTTIType* ResolveFlatType(RED4ext::CName aName);
    const RED4ext::CBaseRTTIType* ResolveFlatType(App::TweakBatch& aBatch, RED4ext::TweakDBID aFlatId);
    const RED4ext::CClass* ResolveRecordType(const YAML::Node& aNode);
    const RED4ext::CClass* ResolveRecordType(App::TweakBatch& aBatch, RED4ext::TweakDBID aRecordId);
    RED4ext::TweakDBID ResolveTweakDBID(const YAML::Node& aNode);

    void ConvertLegacyNodes();

    YAML::Node m_yaml;
    std::filesystem::path m_path;
    TweakDB::Manager& m_manager;
    TweakDB::Reflection& m_reflection;
    App::YamlConverter m_converter;
};
}
