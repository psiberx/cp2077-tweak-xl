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
    RedReader(Core::SharedPtr<Red::TweakDBManager> aManager);
    ~RedReader() override = default;

    bool Load(const std::filesystem::path& aPath) override;
    [[nodiscard]] bool IsLoaded() const override;
    void Unload() override;
    void Read(TweakChangeset& aChangeset) override;

private:
    struct GroupState
    {
        bool isResolved{};
        bool isCompatible{};
        bool isRedefined{};
        bool isRecord{};
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

    GroupStatePtr HandleGroup(App::TweakChangeset& aChangeset, const Red::TweakGroupPtr& aGroup,
                              const std::string& aParentName, const std::string& aParentPath);

    GroupStatePtr HandleInline(App::TweakChangeset& aChangeset, const Red::TweakGroupPtr& aGroup,
                               const std::string& aParentName, const std::string& aParentPath,
                               const Red::CClass* aRequiredType, int32_t aInlineIndex = 0);

    FlatStatePtr HandleFlat(App::TweakChangeset& aChangeset, const Red::TweakFlatPtr & aFlat,
                            const std::string& aParentName, const std::string& aParentPath,
                            const Red::CBaseRTTIType* aRequiredType = nullptr,
                            const Red::CClass* aForeignType = nullptr);

    GroupStatePtr ResolveGroupState(App::TweakChangeset& aChangeset, const Red::TweakGroupPtr& aGroup,
                                    const std::string& aParentName, const std::string& aParentPath,
                                    const Red::CClass* aBaseType = nullptr, int32_t aInlineIndex = 0);

    FlatStatePtr ResolveFlatState(App::TweakChangeset& aChangeset, const Red::TweakFlatPtr& aFlat,
                                  const std::string& aParentName, const std::string& aParentPath,
                                  const Red::CBaseRTTIType* aRequiredType = nullptr,
                                  const Red::CClass* aForeignType = nullptr);

    Red::CName GetFlatTypeName(const Red::TweakFlatPtr& aFlat);

    Red::ValuePtr<> MakeValue(const FlatStatePtr& aState, const Red::TweakValuePtr& aValue);
    Red::ValuePtr<> MakeValue(const FlatStatePtr& aState, const Core::Vector<Red::TweakValuePtr>& aValues);

    std::filesystem::path m_path;
    Red::TweakSourcePtr m_source;
};
}
