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
    : public App::ITweakReader
    , public Core::LoggingAgent
{
public:
    YamlReader(Core::SharedPtr<Red::TweakDBManager> aManager);
    ~YamlReader() override = default;

    bool Load(const std::filesystem::path& aPath) override;
    bool IsLoaded() const override;
    void Unload() override;
    void Read(TweakChangeset& aChangeset) override;

private:
    enum class PropertyMode
    {
        Strict,
        Auto
    };

    static PropertyMode ResolvePropertyMode(const YAML::Node& aNode, PropertyMode aDefault = PropertyMode::Strict);

    void HandleTopNode(TweakChangeset& aChangeset, PropertyMode aPropMode, const std::string& aName,
                       const YAML::Node& aNode);
    void HandleFlatNode(TweakChangeset& aChangeset, const std::string& aName, const YAML::Node& aNode,
                        const Red::CBaseRTTIType* aType = nullptr);
    void HandleRecordNode(TweakChangeset& aChangeset, PropertyMode aPropMode, const std::string& aPath,
                          const std::string& aName, const YAML::Node& aNode, const Red::CClass* aType,
                          Red::TweakDBID aSourceId = 0);
    bool ResolveInlineNode(App::TweakChangeset& aChangeset, const std::string& aPath, const YAML::Node& aNode,
                           const Red::CClass*& aForeignType, Red::TweakDBID& aSourceId);
    bool HandleRelativeChanges(TweakChangeset& aChangeset, const std::string& aPath, const std::string& aName,
                               const YAML::Node& aNode, const Red::CBaseRTTIType* aElementType);
    static bool IsRelativeChange(const YAML::Node& aNode);

    const Red::CBaseRTTIType* ResolveFlatType(const YAML::Node& aNode);
    const Red::CBaseRTTIType* ResolveFlatType(Red::CName aName);
    const Red::CBaseRTTIType* ResolveFlatType(TweakChangeset& aChangeset, Red::TweakDBID aFlatId);
    const Red::CClass* ResolveRecordType(const YAML::Node& aNode);
    const Red::CClass* ResolveRecordType(TweakChangeset& aChangeset, Red::TweakDBID aRecordId);
    Red::TweakDBID ResolveTweakDBID(const YAML::Node& aNode);

    void ConvertLegacyNodes();

    std::filesystem::path m_path;
    YAML::Node m_data;
    Core::SharedPtr<Red::TweakDBManager> m_manager;
    Core::SharedPtr<Red::TweakDBReflection> m_reflection;
    YamlConverter m_converter;
};
}
