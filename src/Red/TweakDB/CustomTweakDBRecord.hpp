#pragma once

#include "Red/TweakDB/Reflection.hpp"

namespace Red
{

class CustomTweakDBRecord : public Red::gamedataTweakDBRecord
{
public:
    static constexpr auto NAME = "gamedataCustomTweakDBRecord";
    static constexpr auto ALIAS = "CustomTweakDBRecord";

    using TYPE = Red::ClassLocator<CustomTweakDBRecord>;

    CustomTweakDBRecord() = default;
    CustomTweakDBRecord(const Red::TweakDBRecordInfo&, Red::TweakDBID);

    void sub_108() override;
    [[nodiscard]] uint32_t GetTweakBaseHash() const override;
    [[nodiscard]] RED4ext::CClass* GetType() override;

    RTTI_IMPL_TYPEINFO(Red::CustomTweakDBRecord);
    RTTI_IMPL_ALLOCATOR();

private:
    const uint32_t m_tweakBaseHash{};
    Red::CClass* m_type{};
};

RED4EXT_ASSERT_SIZE(CustomTweakDBRecord, 0x58);

} // namespace Red

RTTI_DEFINE_CLASS(Red::CustomTweakDBRecord, Red::CustomTweakDBRecord::NAME, {
    // TODO: mark abstract when the CET fix is merged
    RTTI_ALIAS(Red::CustomTweakDBRecord::ALIAS);
    RTTI_PARENT(Red::gamedataTweakDBRecord);
});
