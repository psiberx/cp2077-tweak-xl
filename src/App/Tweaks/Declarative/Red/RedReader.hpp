#pragma once

#include "App/Tweaks/Declarative/TweakReader.hpp"
#include "Core/Logging/LoggingAgent.hpp"
#include "Red/TweakDB/Source/Source.hpp"
#include "Red/Value.hpp"

namespace App
{
class RedReader
    : public BaseTweakReader
    , public Core::LoggingAgent
{
public:
    explicit RedReader(const Core::DeferredPtr<Red::TweakDBManager>& aManager,
                       const Core::DeferredPtr<Red::TweakDBReflection>& aReflection,
                       const Core::SharedPtr<ScriptableRecordManager>& aRecordManager,
                       const Core::SharedPtr<TweakContext>& aContext);
    ~RedReader() override = default;

    bool Load(const std::filesystem::path& aPath) override;
    [[nodiscard]] bool IsLoaded() const override;
    void Unload() override;
    void ReadSchemas(SchemaChangeset& aChangeset) override;
    void ReadValues(TweakChangeset& aChangeset) override;

    static Red::CName GetFlatTypeName(const Red::TweakFlatPtr& aFlat);

private:
    struct GroupState
    {
        bool isResolved{};
        bool isCompatible{};
        bool isRedefined{};
        bool isRecord{};
        bool isOriginalBase{};
        bool isProcessed{};

        const Red::CClass* requiredType{};
        const Red::CClass* resolvedType{};
        Red::TweakDBID sourceId;

        std::string groupPath;
        std::string groupName;
        Red::TweakDBID recordId;
    };

    struct FlatState
    {
        bool isResolved{};
        bool isCompatible{};
        bool isRedefined{};
        bool isProcessed{};

        bool isArray{};
        bool isForeignKey{};
        const Red::CBaseRTTIType* requiredType{};
        const Red::CClass* requiredKey{};
        const Red::CBaseRTTIType* resolvedType{};
        const Red::CBaseRTTIType* elementType{};
        const Red::CClass* resolvedKey{};

        std::string flatPath;
        std::string flatName;
        Red::TweakDBID flatId;
    };

    using GroupStatePtr = Core::SharedPtr<GroupState>;
    using FlatStatePtr = Core::SharedPtr<FlatState>;

    void HandleSchemaGroup(SchemaChangeset& aChangeset, const Red::TweakGroupPtr& aGroup);

    void HandleSchemaProperty(SchemaChangeset& aChangeset, const std::string& aRecordName,
                              const Red::TweakFlatPtr& aFlat);

    GroupStatePtr HandleGroup(TweakChangeset& aChangeset, const Red::TweakGroupPtr& aGroup,
                              const std::string& aParentName, const std::string& aParentPath);

    GroupStatePtr HandleInline(TweakChangeset& aChangeset, const Red::TweakGroupPtr& aGroup,
                               const std::string& aParentName, const std::string& aParentPath,
                               const Red::CClass* aRequiredType, int32_t aInlineIndex = 0);

    FlatStatePtr HandleFlat(TweakChangeset& aChangeset, const Red::TweakFlatPtr& aFlat, const std::string& aParentName,
                            const std::string& aParentPath, const Red::CBaseRTTIType* aRequiredType = nullptr,
                            const Red::CClass* aForeignType = nullptr);

    GroupStatePtr ResolveGroupState(TweakChangeset& aChangeset, const Red::TweakGroupPtr& aGroup,
                                    const std::string& aParentName, const std::string& aParentPath,
                                    const Red::CClass* aBaseType = nullptr, int32_t aInlineIndex = 0);

    FlatStatePtr ResolveFlatState(TweakChangeset& aChangeset, const Red::TweakFlatPtr& aFlat,
                                  const std::string& aParentName, const std::string& aParentPath,
                                  const Red::CBaseRTTIType* aRequiredType = nullptr,
                                  const Red::CClass* aForeignType = nullptr);

    Red::InstancePtr<> MakeValue(const FlatStatePtr& aState, const Red::TweakValuePtr& aValue);
    Red::InstancePtr<> MakeValue(const FlatStatePtr& aState, const Core::Vector<Red::TweakValuePtr>& aValues = {});
    Red::InstancePtr<> MakeValue(const Red::CBaseRTTIType* aType, const Red::TweakValuePtr& aValue);
    Red::InstancePtr<> MakeValue(const Red::CBaseRTTIType* aType, const Core::Vector<Red::TweakValuePtr>& aValues);

    [[nodiscard]] bool CheckConditions(const Core::Vector<std::string>& aTags) const;

    std::filesystem::path m_path;
    Red::TweakSourcePtr m_source;
};
} // namespace App
