#pragma once

namespace App
{
/**
 * @brief A TweakDB record type that serves as the base class for all scriptable record types. This class provides the
 * necessary structure and functionality for scriptable records to function within TweakDB, and scriptable record types
 * should inherit from this class, either directly or indirectly.
 */
class ScriptableTweakDBRecord : public Red::TweakDBRecord
{
public:
    /**
     * @brief The fully-qualified RTTI name of the class.
     */
    static constexpr auto NAME = "gamedataScriptableTweakDBRecord";

    /**
     * @brief The script alias of the class.
     */
    static constexpr auto ALIAS = "ScriptableTweakDBRecord";

    /**
     * @brief Convenience accessor for the RTTI type of this class.
     */
    using TYPE = Red::ClassLocator<ScriptableTweakDBRecord>;

    /**
     * @brief Unknown function required by Red::TweakDBRecord.
     */
    void sub_108() override;

    /**
     * @brief Retrieves the native RTTI type of this record. The function simply returns the
     * Red::IScriptable::nativeType property, which must be set on the scriptable record instance when it is
     * constructed.
     *
     * @return The native RTTI type of this record, which should correspond to the App::ScriptableRecordClass instance
     * representing the record's type.
     */
    Red::CClass* GetNativeType() override;

    /**
     * @brief Retrieves the Murmur3_32 hash of the record type's short name, which is used for lookups in TweakDB. The
     * function retrieves this hash from the App::ScriptableRecordClass instance representing the record's type, which
     * must be set on the scriptable record instance when it is constructed.
     *
     * @return The Murmur3_32 hash of the record type's short name, used for class type lookups in TweakDB.
     */
    [[nodiscard]] uint32_t GetTweakBaseHash() const override;

    RTTI_MEMBER_ACCESS(App::ScriptableTweakDBRecord);
    RTTI_IMPL_ALLOCATOR();
};

RED4EXT_ASSERT_SIZE(ScriptableTweakDBRecord, 0x48);

} // namespace App

RTTI_DEFINE_CLASS(App::ScriptableTweakDBRecord, App::ScriptableTweakDBRecord::NAME, {
    RTTI_ABSTRACT();
    RTTI_ALIAS(App::ScriptableTweakDBRecord::ALIAS);
    RTTI_PARENT(Red::gamedataTweakDBRecord);
});
