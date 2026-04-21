#pragma once

namespace Red
{

class RecordClass;

class ScriptableTweakDBRecord : public Red::gamedataTweakDBRecord
{
public:
    static constexpr auto NAME = "gamedataScriptableTweakDBRecord";
    static constexpr auto ALIAS = "ScriptableTweakDBRecord";

    using TYPE = Red::ClassLocator<ScriptableTweakDBRecord>;

    ScriptableTweakDBRecord() = default;
    explicit ScriptableTweakDBRecord(const Red::RecordClass* aClass);

    void sub_108() override;
    Red::CClass* GetNativeType() override;
    [[nodiscard]] uint32_t GetTweakBaseHash() const override;

    RTTI_MEMBER_ACCESS(Red::ScriptableTweakDBRecord);
    RTTI_IMPL_ALLOCATOR();
};

RED4EXT_ASSERT_SIZE(ScriptableTweakDBRecord, 0x48);

class RecordClass : public Red::CClass
{
public:
    static constexpr auto ScriptableRecordSize = sizeof(ScriptableTweakDBRecord);
    static constexpr auto ScriptableRecordAlignment = alignof(ScriptableTweakDBRecord);

    RecordClass(CName aName, const uint32_t aHash)
        : CClass(aName, ScriptableRecordSize, {.isNative = true})
        , tweakBaseHash(aHash)
    {
        alignment = ScriptableRecordAlignment;
    }

    void ConstructCls(void* aMemory) const override
    {
        new (aMemory) ScriptableTweakDBRecord(this);
    }

    void DestructCls(void* aMemory) const override
    {
        static_cast<ScriptableTweakDBRecord*>(aMemory)->~ScriptableTweakDBRecord();
    }

    void* AllocMemory() const override
    {
        const auto alignedSize = AlignUp(size, alignment);

        const auto allocator = GetAllocator();
        auto [memory, size] = allocator->AllocAligned(alignedSize, alignment);

        std::memset(memory, 0, size);
        return memory;
    }

    const bool IsEqual(const void* aLhs, const void* aRhs, uint32_t a3) override
    {
        using func_t = bool (*)(CClass*, const void*, const void*, uint32_t);
#if defined(RED4EXT_V1_SDK_VERSION_CURRENT) || defined(RED4EXT_SDK_0_5_0)
        static UniversalRelocFunc<func_t> func(RED4ext::Detail::AddressHashes::TTypedClass_IsEqual);
#else
        static RelocFunc<func_t> func(RED4ext::Addresses::TTypedClass_IsEqual);
#endif
        return func(this, aLhs, aRhs, a3);
    }

    void Assign(void* aLhs, const void* aRhs) const override
    {
        new (aLhs) ScriptableTweakDBRecord(*static_cast<const ScriptableTweakDBRecord*>(aRhs));
    }

    const uint32_t tweakBaseHash;
};

} // namespace Red

RTTI_DEFINE_CLASS(Red::ScriptableTweakDBRecord, Red::ScriptableTweakDBRecord::NAME, {
    // TODO: mark abstract when the CET fix is merged
    RTTI_ALIAS(Red::ScriptableTweakDBRecord::ALIAS);
    RTTI_PARENT(Red::gamedataTweakDBRecord);
});
