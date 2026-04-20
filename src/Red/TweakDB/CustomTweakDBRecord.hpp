#pragma once

namespace Red
{

class CustomTweakDBRecord : public Red::gamedataTweakDBRecord
{
public:
    static constexpr auto NAME = "gamedataCustomTweakDBRecord";
    static constexpr auto ALIAS = "CustomTweakDBRecord";

    using TYPE = Red::ClassLocator<CustomTweakDBRecord>;

    CustomTweakDBRecord() = default;

    void sub_108() override;
    [[nodiscard]] uint32_t GetTweakBaseHash() const override;

    RTTI_IMPL_TYPEINFO(Red::CustomTweakDBRecord);
    RTTI_IMPL_ALLOCATOR();

    uint32_t tweakBaseHash{};
};

RED4EXT_ASSERT_SIZE(CustomTweakDBRecord, 0x50);

} // namespace Red

RTTI_DEFINE_CLASS(Red::CustomTweakDBRecord, Red::CustomTweakDBRecord::NAME, {
    // TODO: mark abstract when the CET fix is merged
    RTTI_ALIAS(Red::CustomTweakDBRecord::ALIAS);
    RTTI_PARENT(Red::gamedataTweakDBRecord);
});
