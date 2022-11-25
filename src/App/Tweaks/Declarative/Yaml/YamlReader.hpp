#pragma once

#include "App/Tweaks/Batch/TweakChangeset.hpp"
#include "App/Tweaks/Declarative/TweakReader.hpp"
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
    bool HandleMutations(TweakChangeset& aChangeset, const std::string& aPath, const std::string& aName,
                         const YAML::Node& aNode, const Red::CBaseRTTIType* aElementType);

    static PropertyMode ResolvePropertyMode(const YAML::Node& aNode, PropertyMode aDefault = PropertyMode::Strict);
    const Red::CBaseRTTIType* ResolveFlatType(const YAML::Node& aNode);
    const Red::CBaseRTTIType* ResolveFlatType(Red::CName aName);
    const Red::CClass* ResolveRecordType(const YAML::Node& aNode);
    Red::TweakDBID ResolveTweakDBID(const YAML::Node& aNode);

    template<typename T>
    Red::InstancePtr<T> ConvertValue(const YAML::Node& aNode, bool aStrict = false);

    template<typename T>
    bool ConvertValue(const YAML::Node& aNode, Red::InstancePtr<>& aValue, bool aStrict = false);

    template<typename E>
    Red::InstancePtr<Red::DynArray<E>> ConvertArray(const YAML::Node& aNode, bool aStrict = false);

    template<typename E>
    bool ConvertArray(const YAML::Node& aNode, Red::InstancePtr<>& aValue, bool aStrict = false);

    Red::InstancePtr<> MakeValue(Red::CName aTypeName, const YAML::Node& aNode);
    Red::InstancePtr<> MakeValue(const Red::CBaseRTTIType* aType, const YAML::Node& aNode);
    std::pair<Red::CName, Red::InstancePtr<>> TryMakeValue(const YAML::Node& aNode);

    void ConvertLegacyNodes();

    std::filesystem::path m_path;
    YAML::Node m_data;
};
}
