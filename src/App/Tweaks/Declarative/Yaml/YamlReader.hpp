#pragma once

#include "App/Tweaks/Batch/TweakChangeset.hpp"
#include "App/Tweaks/Declarative/TweakReader.hpp"
#include "App/Tweaks/Declarative/Yaml/YamlConverter.hpp"
#include "Core/Logging/LoggingAgent.hpp"

namespace App
{
class YamlReader
    : public BaseTweakReader
    , public Core::LoggingAgent
{
public:
    YamlReader(Core::SharedPtr<Red::TweakDBManager> aManager);
    ~YamlReader() override = default;

    bool Load(const std::filesystem::path& aPath) override;
    [[nodiscard]] bool IsLoaded() const override;
    void Unload() override;
    void Read(TweakChangeset& aChangeset) override;

private:
    enum class PropertyMode
    {
        Strict,
        Auto
    };

    void HandleTopNode(TweakChangeset& aChangeset, PropertyMode aPropMode, const std::string& aName,
                       const YAML::Node& aNode);
    void HandleFlatNode(TweakChangeset& aChangeset, const std::string& aName, const YAML::Node& aNode,
                        const Red::CBaseRTTIType* aType = nullptr);
    void HandleRecordNode(TweakChangeset& aChangeset, PropertyMode aPropMode, const std::string& aRecordPath,
                          const std::string& aRecordName, const YAML::Node& aNode, const Red::CClass* aRecordType,
                          Red::TweakDBID aSourceId = 0);
    bool ResolveInlineNode(App::TweakChangeset& aChangeset, const std::string& aPath, const YAML::Node& aNode,
                           const Red::CClass*& aForeignType, Red::TweakDBID& aSourceId);
    bool HandleRelativeChanges(TweakChangeset& aChangeset, const std::string& aPath, const std::string& aName,
                               const YAML::Node& aNode, const Red::CBaseRTTIType* aElementType);
    static bool IsRelativeChange(const YAML::Node& aNode);
    static PropertyMode ResolvePropertyMode(const YAML::Node& aNode, PropertyMode aDefault = PropertyMode::Strict);

    const Red::CBaseRTTIType* ResolveFlatType(const YAML::Node& aNode);
    const Red::CBaseRTTIType* ResolveFlatType(Red::CName aName);
    const Red::CClass* ResolveRecordType(const YAML::Node& aNode);
    Red::TweakDBID ResolveTweakDBID(const YAML::Node& aNode);

    void ConvertLegacyNodes();

    std::filesystem::path m_path;
    YAML::Node m_data;
    YamlConverter m_converter;
};
}
