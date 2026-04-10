#pragma once

#include "App/Tweaks/Record/ScriptableRecordClass.hpp"
#include "App/Tweaks/TweakTypeSpec.hpp"
#include "Red/TweakDB/Manager.hpp"

namespace App
{
/**
 * @brief A convenience alias for an array of weak handles to TweakDB records.
 */
using RecordArray = Red::DynArray<Red::WeakHandle<Red::TweakDBRecord>>;

/**
 * @brief A convenience alias for a weak handle to a TweakDB record.
 */
using RecordWHandle = Red::WeakHandle<Red::TweakDBRecord>;

/**
 * @brief A convenience alias for a handle to a TweakDB record.
 */
using RecordHandle = Red::Handle<Red::TweakDBRecord>;

/**
 * @brief Defines the specification of a property of a scriptable record.
 */
struct ScriptablePropertySpec
{
    /**
     * @brief The name of the property.
     */
    std::string name;

    /**
     * @brief The base function name of the property. This is the same as @c name but with the first letter capitalized,
     * if it was not already.
     */
    std::string functionName;

    /**
     * @brief The computed name of @c name, which is used for indexing and lookups.
     */
    Red::CName cname;

    /**
     * @brief The string to append to a scriptable record's TweakDB ID to retrieve the flat value of the property. This
     * is the same as @c name but prepended with a period.
     */
    std::string appendix;

    /**
     * @brief The type specification of the property.
     */
    TweakTypeSpecPtr typeSpec;

    /**
     * @brief The default value of the property, if it has one. Instances of the scriptable record that do not define an
     * explicit value for the property will inherit this value. If not provided, the default value of the property will
     * be the "empty" value of the corresponding type.
     */
    Red::InstancePtr<> defaultValue;

    /**
     * @brief Whether the property and its functions have been created.
     */
    bool isCreated = false;

    /**
     * @brief Whether the property and its functions have been soft-deleted.
     */
    bool isDeleted = false;

    /**
     * Whether the current default value has been inserted as a flat value into the RTDB namespace of TweakDB.
     */
    bool isInserted = false;
};

/**
 * @brief A convenience alias for a shared pointer to a scriptable property specification.
 */
using ScriptablePropertySpecPtr = Core::SharedPtr<ScriptablePropertySpec>;

/**
 * @brief Defines the specification of a scriptable record, which includes its properties and metadata.
 */
struct ScriptableRecordSpec
{
    /**
     * @brief Find a property specification by the base name of its corresponding function.
     *
     * @param aFunctionName The base name of the function corresponding to the property. For example, if the property is
     * "health" and its corresponding function is "GetHealth", then the base name would be "Health".
     * @return A shared pointer to the property specification if found, or @c nullptr if no property with the given
     * function name exists.
     */
    [[nodiscard]] ScriptablePropertySpecPtr FindPropertyByFunctionName(const std::string& aFunctionName) const;

    /**
     * @brief Find a property specification by the name of the property.
     *
     * @param aName The name of the property to find.
     * @return A shared pointer to the property specification if found, or @c nullptr if no property with the given name
     * exists.
     */
    [[nodiscard]] ScriptablePropertySpecPtr FindPropertyByName(const std::string& aName) const;

    /**
     * @brief The fully-qualified, or native, name of the scriptable record.
     */
    std::string name;

    /**
     * @brief The alias name, or script name, of the scriptable record.
     */
    std::string aliasName;

    /**
     * @brief The short name of the scriptable record.
     */
    std::string shortName;

    /**
     * @brief The computed name of @c name, which is used for indexing and lookups.
     */
    Red::CName cname;

    /**
     * @brief The computed name of @c aliasName, which is used for indexing and lookups.
     */
    Red::CName aliasCName;

    /**
     * @brief The computed name of @c shortName, which is used for indexing and lookups.
     */
    Red::CName shortCName;

    /**
     * @brief The Murmur32_2 hash of the scriptable record's short name, which is used for lookups when creating new
     * instances of the record.
     */
    uint32_t hash;

    /**
     * @brief The class of the scriptable record.
     */
    ScriptableRecordClass* type;

    /**
     * @brief The optional name of the parent scriptable record, if any. If not provided, the record will directly
     * inherit from @c App::ScriptableTweakDBRecord .
     */
    std::optional<std::string> parent;

    /**
     * @brief A map of property specifications belonging to the scriptable record, indexed by the computed name of the
     * property.
     */
    Core::Map<Red::CName, ScriptablePropertySpecPtr> props;

    /**
     * @brief Whether the scriptable record has been registered with RTTI.
     */
    bool isRegistered = false;

    /**
     * @brief Whether the scriptable record has been described with RTTI.
     */
    bool isDescribed = false;

    /**
     * @brief Whether the scriptable record and its properties have been soft-deleted.
     */
    bool isDeleted = false;
};

/**
 * @brief A convenience alias for a shared pointer to a scriptable record specification.
 */
using ScriptableRecordSpecPtr = Core::SharedPtr<ScriptableRecordSpec>;

} // namespace App
