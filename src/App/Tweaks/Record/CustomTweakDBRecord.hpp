#pragma once
#include "Red/TweakDB/Reflection.hpp"

namespace App
{

class CustomTweakDBRecord : public Red::gamedataTweakDBRecord
{
public:
    static constexpr auto NAME = "gamedataCustomTweakDBRecord";
    static constexpr auto ALIAS = "CustomTweakDBRecord";
    static constexpr Red::ClassLocator<CustomTweakDBRecord> TYPE;

    CustomTweakDBRecord() = default;
    CustomTweakDBRecord(const Red::TweakDBRecordInfo&, Red::TweakDBID);

    void sub_108() override;
    [[nodiscard]] uint32_t GetTweakBaseHash() const override;
    [[nodiscard]] RED4ext::CClass* GetType() override;

    RTTI_IMPL_TYPEINFO(App::CustomTweakDBRecord);
    RTTI_IMPL_ALLOCATOR();

private:
    const uint32_t m_tweakBaseHash{};
    Red::CClass* m_type{};
};

RED4EXT_ASSERT_SIZE(CustomTweakDBRecord, 0x58);

} // namespace App

RTTI_DEFINE_CLASS(App::CustomTweakDBRecord, App::CustomTweakDBRecord::NAME, {
    RTTI_ALIAS(App::CustomTweakDBRecord::ALIAS);
    RTTI_PARENT(Red::gamedataTweakDBRecord);
});
