#pragma once

namespace App
{
/**
 * @brief A custom RTTI class that represents a scriptable record type in TweakDB.
 *
 * Every scriptable record type relies on having its own instance of this class to manage its lifecycle and provide
 * class hash information necessary for type lookups in TweakDB.
 */
class ScriptableRecordClass : public Red::CClass
{
public:
    /**
     * @brief The allocator type used for instances of this class.
     */
    using AllocatorType = Red::Memory::RTTIAllocator;

    /**
     * @brief Constructs a new ScriptableRecordClass with the specified name and hash.
     *
     * The name of the record must be registered in the CName pool as the fully-qualified name of the record type,
     * following the convention of @c gamedata{RecordName}_Record . The hash is the Murmur3_32 hash of the record's
     * short name (the fully-qualified name without the @c gamedata prefix and @c _Record suffix).
     *
     * By default, the constructed class will be marked as native.
     *
     * @param aName The fully-qualified name of the record type, which must be registered in the CName pool.
     * @param aHash The Murmur3_32 hash of the record's short name, used for type lookups in TweakDB.
     */
    ScriptableRecordClass(RED4ext::CName aName, uint32_t aHash);

    /**
     * @brief Constructs an instance of the scriptable record class in the provided memory location.
     *
     * After construction, the scriptable record instance must have its native type and record ID assigned.
     *
     * @param aMemory A pointer to the memory location where the instance should be constructed. The memory must be
     * properly aligned and large enough to hold an instance of the scriptable record.
     */
    void ConstructCls(void* aMemory) const override;

    /**
     * @brief Destructs and frees an instance of the scriptable record class at the provided memory location.
     *
     * @param aMemory A pointer to the memory location where the instance is located.
     */
    void DestructCls(void* aMemory) const override;

    /**
     * @brief Allocates memory for an instance of the scriptable record class, ensuring proper alignment and size.
     *
     * @return A pointer to the allocated memory, which is zero-initialized. The caller is responsible for ensuring that
     * the memory is properly managed after creation so that it does not leak.
     */
    [[nodiscard]] void* AllocMemory() const override;

    /**
     * @brief Compares the values of two instances of the scriptable record class for equality.
     *
     * @param aLhs Pointer to the scriptable record instance on the left-hand side of the equality comparison.
     * @param aRhs Pointer to the scriptable record instance on the right-hand side of the equality comparison.
     * @param a3 Unused
     * @return Whether the two instances are considered equal.
     */
    const bool IsEqual(const void* aLhs, const void* aRhs, uint32_t a3) override;

    /**
     * @brief Assigns (copies) the value of one instance of the scriptable record class to another instance.
     *
     * @param aLhs Pointer to the scriptable record instance that will receive the assigned value (the left-hand side of
     * the assignment).
     * @param aRhs Pointer to the scriptable record instance that will be copied from (the right-hand side of the
     * assignment).
     */
    void Assign(void* aLhs, const void* aRhs) const override;

    /**
     * @brief The Murmur3_32 hash of the record's short name, used for type lookups in TweakDB.
     */
    const uint32_t tweakBaseHash;
};
} // namespace App
