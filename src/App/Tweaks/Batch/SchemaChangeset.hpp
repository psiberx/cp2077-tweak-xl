#pragma once
#include "App/Tweaks/Record/ScriptableRecordManager.hpp"
#include "App/Tweaks/TweakTypeSpec.hpp"
#include "Core/Logging/LoggingAgent.hpp"

namespace App
{
/**
 * @brief Represents a pending changes to scriptable record schemas and their properties, including additions, removals,
 * and modifications.
 *
 * After the initial load, changes to scriptable record schemas are much more limited than Tweak values. Record types
 * may be added or removed, however removed record types are merely soft-deleted to ensure that there are no dangling
 * references in the scripting layer. Functions that operate on properties of scriptable record types that are soft
 * deleted are modified so that their script logic is effectively a no-op.
 *
 * Individual properties of scriptable record type may also be added or deleted (with the same soft-delete mechanics),
 * however their type may not be changed after initial load. Their default values may be updated.
 *
 */
class SchemaChangeset
    : public Core::LoggingAgent
    , public Core::ShareFromThis<SchemaChangeset>
{
public:
    /**
     * @brief Represents a pending change to a property of a scriptable record, including the property's name, type, and
     * default value.
     */
    struct PropertyEntry
    {
        /**
         * @brief The name of the property.
         */
        std::string name;

        /**
         * @brief The type specification of the property.
         */
        TweakTypeSpecPtr type;

        /**
         * @brief The default value of the property.
         */
        Red::InstancePtr<> defaultValue;
    };

    /**
     * @brief Represents a pending change to a scriptable record type, including the record's name, optional parent
     * record, and properties.
     */
    struct RecordEntry
    {
        /**
         * @brief The normalized name of the record type as parsed from a Tweak file.
         */
        std::string name;

        /**
         * @brief The optional, unmodified name of the parent record type as parsed from a Tweak file. If not provided,
         * the record will directly inherit from @c App::ScriptableTweakDBRecord.
         */
        std::optional<std::string> parent;

        /**
         * @brief A map of pending changes to properties belonging to the scriptable record, indexed by the computed
         * name of the property.
         */
        Core::Map<Red::CName, PropertyEntry> properties;
    };

    /**
     * @brief Creates a pending change to add a new scriptable record type with the given name and optional parent
     * record.
     *
     * @param aName The unmodified name of the record type as parsed from a Tweak file.
     * @param aParent The optional, unmodified name of the parent record type as parsed from a Tweak file.
     * @return true if the record was successfully added to the changeset, otherwise false.
     */
    bool MakeRecord(const std::string& aName, const std::optional<std::string>& aParent);

    /**
     * @brief Creates a pending change to add a new property to a scriptable record type with the given name, type, and
     * default value.
     *
     * @param aRecordName The unmodified name of the record type as parsed from a Tweak file to which the property
     * belongs.
     * @param aPropName The name of the property.
     * @param aTypeInfo The type specification of the property.
     * @param aDefaultValue The default value of the property.
     * @return true if the property was successfully added to the changeset, otherwise false.
     */
    bool MakeProperty(const std::string& aRecordName, const std::string& aPropName, const TweakTypeSpecPtr& aTypeInfo,
                      const Red::InstancePtr<>& aDefaultValue);

    /**
     * @brief Commits the pending changes in the changeset to the given record manager, registering new record types and
     * properties, soft-deleting removed record types and properties, and updating default values of existing
     * properties.
     *
     * @param aRecordManager The record manager to which the changes should be committed.
     */
    void Commit(const Core::SharedPtr<ScriptableRecordManager>& aRecordManager);

    /**
     * @brief Checks whether the changeset is empty, i.e. contains no pending changes to scriptable record types or
     * their properties.
     *
     * @return true if the changeset is empty, otherwise false.
     */
    bool IsEmpty() const;

private:
    /**
     * @brief Checks whether the given values of a TweakDB property are equal, taking into account the property's type.
     * This is used to determine whether the default value of a property has changed and needs to be updated in the
     * scripting layer when committing changes to existing properties.
     *
     * @param aType The RTTI type of the property to which the values belong.
     * @param aLhs The first value to compare.
     * @param aRhs The second value to compare.
     * @return true if the values are considered equal, otherwise false.
     */
    static bool IsEqual(const Red::CBaseRTTIType* aType, const Red::InstancePtr<>& aLhs,
                        const Red::InstancePtr<>& aRhs);

    /**
     * @brief A map of pending changes to scriptable record types, indexed by the computed full name of the record type.
     */
    Core::Map<Red::CName, RecordEntry> m_pendingRecords;
};
} // namespace App
