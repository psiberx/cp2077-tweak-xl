#pragma once

namespace App
{
/**
 * @brief Contains a specification for App::ScriptableTweakDBRecord property, including its TweakDB flat type and getter
 * closure return type details.
 */
struct TweakTypeSpec
{
    /**
     * @brief The RTTI type that should be returned from the property's base getter closure (i.e. the function that
     * returns its value). In most cases, the property type and flat type will match, however there a few exceptions.
     *
     * Critically, this property should not be used for TweakDBID, or foreign key, types. If @c isForeignKey is @c true,
     * the functions that interact with the flat value will need to construct the appropriate return type by referencing
     * the @c foreignType property of this class.
     *
     * Note that if @c isArray is @ true, this type is guaranteed to be castable to @c CRTTIBaseArrayType .
     */
    const Red::CBaseRTTIType* propertyType{};

    /**
     * @brief The name of the RTTI type pointed to by @c propertyType .
     */
    Red::CName propertyTypeName{};

    /**
     * @brief The RTTI type of the TweakDB flat that corresponds to this property.
     */
    const Red::CBaseRTTIType* flatType{};

    /**
     * @brief The name of the RTTI type of the TweakDB flat that corresponds to this property.
     */
    Red::CName flatTypeName;

    /**
     * @brief Whether this property is an array type.
     */
    bool isArray{};

    /**
     * @brief Whether this property represents a foreign key reference to another TweakDB record. If true, the flat type
     * will be either a TweakDBID or array of TweakDBIDs. Getter functions associated with foreign key properties return
     * handles to TweakDB records and should reference the @c foreignKey property of this class for the RTTI class type
     * of the target record.
     */
    bool isForeignKey{};

    /**
     * @brief Whether this property represents a ResRef or an array of ResRefs. If true, the flat type will be either a
     * RaRef or an array of RaRefs, and the getter functions will return a ResRef or an array of ResRefs, respectively.
     */
    bool isResRef{};

    /**
     * @brief Whether this property represents a localization key or an array of localization keys.
     */
    bool isLocKey{};

    /**
     * @brief The name of the foreign type as specified by a YAML or Red Tweak file before conversion to a
     * fully-qualified TweakDB record name. Useful for logging errors related to unresolved foreign types when parsing
     * TweakDB properties from YAML or Red Tweak files.
     */
    std::string foreignName;

    /**
     * @brief The RTTI type of the referenced record for foreign key properties. This initially may be null if the
     * referenced type is a scriptable record that has not yet been created and registered with RTTI.
     *
     * During initialization, the foreign type will be resolved and validated. If it cannot be resolved, the scriptable
     * record property should not be created.
     */
    const Red::CClass* foreignType{};

    /**
     * @brief The name of the referenced record for foreign key properties, used for type resolution when the referenced
     * type is not yet available (e.g. for scriptable record foreign keys that have not yet been created).
     */
    Red::CName foreignTypeName;

    /**
     * @brief Checks whether this TweakTypeSpec is equal to another by comparing all members for equality.
     *
     * @param aOther The other TweakTypeSpec to compare against.
     * @return true if all members of both TweakTypeSpecs are equal, otherwise false.
     */
    bool operator==(const TweakTypeSpec& aOther) const = default;

    /**
     * @brief Checks whether this TweakTypeSpec is not equal to another by comparing all members for inequality.
     * @param aOther The other TweakTypeSpec to compare against.
     * @return true if any member of the TweakTypeSpecs is not equal, otherwise false.
     */
    bool operator!=(const TweakTypeSpec& aOther) const
    {
        return !(*this == aOther);
    }
};

/**
 * @brief A shared pointer to a TweakTypeSpec struct.
 */
using TweakTypeSpecPtr = Core::SharedPtr<TweakTypeSpec>;

/**
 * @brief Parses the given string and infers the TweakDB property and flat details for a scriptable record. This is
 * useful for parsing types discovered from YAML record schemas and supports all valid TweakDB RTTI types, including
 * foreign keys and arrays of foreign keys to other TweakDB record types.
 *
 * See Red::ERTDBFlatType for a list of all supported TweakDB flat type names. Getter functions of foreign key types may
 * be represented as the foreign record's class name (full, script alias, or short) for properties pointing to a single
 * foreign key or prefixed with "array:" for an array of foreign keys.
 *
 * If the given value represents a foreign key flat type and a foreign type name is not provided, no type spec will be
 * returned.
 *
 * @param aValue The string to parse TweakDB property and flat details from.
 * @return A property spec containing the parsed property and flat details, or nullptr if the given string is not
 * a valid TweakDB property type.
 */
TweakTypeSpecPtr GetTweakTypeSpec(const std::string& aValue);

/**
 * @brief Parses the given string and infers the TweakDB property and flat details for a scriptable record. This is
 * useful for parsing types discovered from YAML record schemas and supports all valid TweakDB RTTI types, including
 * foreign keys and arrays of foreign keys to other TweakDB record types.
 *
 * See Red::ERTDBFlatType for a list of all supported TweakDB flat type names. Getters of foreign key types may be
 * represented as the foreign record's class name (full, script alias, or short) for properties pointing to a single
 * foreign key or prefixed with "array:" for an array of foreign keys.
 *
 * If the given value represents a foreign key flat type and a foreign type name is not provided, no type spec will be
 * returned.
 *
 * @param aValue The string to parse TweakDB property and flat details from.
 * @return A property spec containing the parsed property and flat details, or nullptr if the given string is not
 * a valid TweakDB property type.
 */
TweakTypeSpecPtr GetTweakTypeSpec(const char* aValue);

/**
 * @brief Creates a set of details for a TweakDB property based on the given type name and optional foreign type name.
 *
 * If the given name represents a foreign key flat type and a foreign type name is not provided, no type spec will be
 * returned.
 *
 * @param aName The name of the property type, used for RTTI type resolution.
 * @param aForeignType An optional foreign type name for foreign key properties.
 * @return A property spec containing the parsed property and flat details, or nullptr if the given string is not
 * a valid TweakDB property type.
 * @see Red::ERTDBFlatType
 */
TweakTypeSpecPtr GetTweakTypeSpec(Red::CName aName, const std::optional<std::string>& aForeignType = std::nullopt);

/**
 * @brief Creates a set of details for a TweakDB property based on the given type hash and optional foreign type name.
 *
 * If the given type represents a foreign key relationship and a foreign type name is not provided, no type spec will be
 * returned.
 *
 * @tparam Type The hash of the property type, used for RTTI type resolution.
 * @param aValue An optional foreign type name for foreign key properties.
 * @return A property spec containing the parsed property and flat details, or nullptr if not valid.
 * @see Red::ERTDBFlatType
 */
template<uint64_t Type>
TweakTypeSpecPtr GetTweakTypeSpec(const std::optional<std::string>& aValue = std::nullopt)
{
    static const Core::SharedPtr<TweakTypeSpec> spec = GetTweakTypeSpec(Type, aValue);
    return spec;
}

} // namespace App
