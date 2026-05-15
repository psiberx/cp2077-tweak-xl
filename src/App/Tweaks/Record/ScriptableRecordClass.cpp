#include "ScriptableRecordClass.hpp"
#include "ScriptableTweakDBRecord.hpp"

namespace
{
constexpr auto ScriptableRecordSize = sizeof(App::ScriptableTweakDBRecord);
constexpr auto ScriptableRecordAlignment = alignof(App::ScriptableTweakDBRecord);
} // namespace

namespace App
{
ScriptableRecordClass::ScriptableRecordClass(Red::CName aName, const uint32_t aHash)
    : CClass(aName, ScriptableRecordSize, {.isNative = true})
    , tweakBaseHash(aHash)
{
    alignment = ScriptableRecordAlignment;
}

void ScriptableRecordClass::ConstructCls(void* aMemory) const
{
    new (aMemory) ScriptableTweakDBRecord();
}

void ScriptableRecordClass::DestructCls(void* aMemory) const
{
    static_cast<ScriptableTweakDBRecord*>(aMemory)->~ScriptableTweakDBRecord();
}

void* ScriptableRecordClass::AllocMemory() const
{
    const auto alignedSize = Red::AlignUp(size, alignment);

    const auto allocator = GetAllocator();
    auto [memory, size] = allocator->AllocAligned(alignedSize, alignment);

    std::memset(memory, 0, size);
    return memory;
}

const bool ScriptableRecordClass::IsEqual(const void* aLhs, const void* aRhs, const uint32_t a3)
{
    using func_t = bool (*)(CClass*, const void*, const void*, uint32_t);
#if defined(RED4EXT_V1_SDK_VERSION_CURRENT) || defined(RED4EXT_SDK_0_5_0)
    static RED4ext::UniversalRelocFunc<func_t> func(RED4ext::Detail::AddressHashes::TTypedClass_IsEqual);
#else
    static RelocFunc<func_t> func(RED4ext::Addresses::TTypedClass_IsEqual);
#endif
    return func(this, aLhs, aRhs, a3);
}

void ScriptableRecordClass::Assign(void* aLhs, const void* aRhs) const
{
    new (aLhs) ScriptableTweakDBRecord(*static_cast<const ScriptableTweakDBRecord*>(aRhs));
}
} // namespace App
