#pragma once

namespace Red
{
/**
 * @brief A struct representing the reflection information of a TweakDB record property, containing its name, type
 * information, and other relevant details required for proper handling of TweakDB record instance and flat operations.
 *
 * These are details are discovered through inspection of functions belonging to TweakDB record class types.
 */
struct TweakDBPropertyInfo
{
    /**
     * Name of the flat value of the property.
     */
    CName name;

    /**
     * @brief The RTTI type of the flat value.
     */
    const CBaseRTTIType* type;

    /**
     * @brief The RTTI type of the elements of the property if it is an array or handle, or nullptr otherwise.
     */
    const CBaseRTTIType* elementType;

    /**
     * @brief The RTTI class type of the foreign record if the property is a foreign key, or nullptr otherwise.
     */
    const CClass* foreignType;

    /**
     * @brief A boolean flag indicating whether the property's flat value is an array type.
     */
    bool isArray;

    /**
     * @brief A boolean flag indicating whether the property is a foreign key to another TweakDB record type.
     */
    bool isForeignKey;

    /**
     * @brief A boolean flag indicating whether the property is an extra flat that is not defined by the original
     * TweakDB record class, but is added through registration.
     */
    bool isExtra;

    /**
     * @brief The appendix used to build the TweakDBID of the property's flat value, which is appended to the record's
     * TweakDBID when retrieving the flat value from TweakDB.
     */
    std::string appendix;

    /**
     * @brief The offset of the default value of the property in the TweakDB flat buffer containing default values for
     * the record type, or an empty optional if the default value offset cannot be found.
     */
    std::optional<int32_t> defaultValue;
};

/**
 * @brief A struct representing the reflection information of a TweakDB record type, containing its name, type
 * information, parent record type, properties, and other relevant details required for proper handling of TweakDB
 * record instance and flat operations.
 */
struct TweakDBRecordInfo
{
    /**
     * @brief The name of the record type, which is the same as the name of the corresponding RTTI class type.
     */
    CName name;

    /**
     * @brief The RTTI class type of the record type corresponding to this record info.
     */
    const CClass* type;

    /**
     * @brief The RTTI class type of the parent record type if this record type is a child in the TweakDB record
     * inheritance hierarchy, or nullptr if this record type is a direct descendent of @c TweakDBRecord .
     */
    const CClass* parent;

    /**
     * @brief A map of property names to their corresponding reflection information for all properties of this record
     * type.
     */
    Core::Map<CName, Core::SharedPtr<TweakDBPropertyInfo>> props;

    /**
     * @brief The short name of the record type, which is derived from the RTTI class name by removing the "gamedata"
     * prefix and "_Record" suffix.
     *
     * For example, the short name of the record type corresponding to the class "gamedataVehicle_Record" would be
     * "Vehicle".
     */
    std::string shortName;

    /**
     * @brief The hash of the record type name, which is used for efficient lookup of record types and their properties.
     * This is derived from the short name of the record type.
     */
    uint32_t typeHash;

    /**
     * @brief Gets the reflection information of the property with the given name if it exists, or nullptr otherwise.
     *
     * @param aPropName The name of the property to get the reflection information for.
     * @return A pointer to the reflection information of the property with the given name if it exists, or nullptr
     * otherwise.
     */
    [[nodiscard]] const TweakDBPropertyInfo* GetPropInfo(CName aPropName) const
    {
        const auto& propIt = props.find(aPropName);
        return propIt != props.end() ? propIt->second.get() : nullptr;
    }
};

/**
 * @brief A class responsible for collecting and providing information about TweakDB record types and their properties
 * through reflection. Additionally, the class manages a cache of relationships between original TweakDB records and
 * their descendants, and handles the registration of extra flats that are not defined by the original TweakDB record
 * classes but are added through registration.
 *
 * This class serves as the central point for all reflection-related operations for TweakDB records, providing a unified
 * interface for accessing record and property information, managing record relationships, and registering extra flats.
 */
class TweakDBReflection
{
public:
    /**
     * @brief Constructs a TweakDBReflection instance with the default TweakDB singleton instance.
     */
    TweakDBReflection();

    /**
     * @brief Constructs a TweakDBReflection instance with the given TweakDB instance.
     *
     * @param aTweakDb A pointer to the TweakDB instance to use for this TweakDBReflection instance.
     */
    explicit TweakDBReflection(TweakDB* aTweakDb);

    /**
     * @brief Gets the reflection information of the TweakDB record type with the given name if it exists, or nullptr
     * otherwise.
     *
     * If the details of the record type have not been collected yet, they will be collected and cached before being
     * returned.
     *
     * @param aTypeName The name of the record type to get the reflection information for.
     * @return A pointer to the reflection information of the TweakDB record type with the given name if it exists, or
     * nullptr otherwise.
     */
    const TweakDBRecordInfo* GetRecordInfo(CName aTypeName);

    /**
     * @brief Gets the reflection information of the TweakDB record type corresponding to the given RTTI class type if
     * it exists, or nullptr otherwise.
     *
     * If the details of the record type have not been collected yet, they will be collected and cached before being
     * returned.
     *
     * @param aType The RTTI class type of the record type to get the reflection information for.
     * @return A pointer to the reflection information of the TweakDB record type corresponding to the given RTTI class
     * type if it exists, or nullptr otherwise.
     */
    const TweakDBRecordInfo* GetRecordInfo(const CClass* aType);

    /**
     * @brief Determines whether the given TweakDB record ID corresponds to an original TweakDB record, which is a
     * record that is built into the game's compiled TweakDB and not added via TweakXL.
     *
     * @param aRecordId The TweakDBID of the record to check.
     * @return true if the given TweakDB record ID corresponds to an original TweakDB record, false otherwise.
     */
    [[nodiscard]] bool IsOriginalRecord(TweakDBID aRecordId) const;

    /**
     * @brief Determines whether the given TweakDB record ID corresponds to an original base record in the TweakDB
     * record inheritance hierarchy.
     *
     * An original record is a TweakDB record instance which is built into the game's compiled TweakDB rather than added
     * via TweakXL.
     *
     * A base record is a TweakDB record instance that has descendant record instances in the TweakDB record inheritance
     * hierarchy, where the descendant record instance may inherit some or all of the properties of the base record
     * instance.
     *
     * @param aParentId The TweakDBID of the record to check whether it is an original base record.
     * @return true if the given TweakDB record ID corresponds to an original base record, false otherwise.
     */
    [[nodiscard]] bool IsOriginalBaseRecord(TweakDBID aParentId) const;

    /**
     * @brief Gets the TweakDBID of the original parent record of the given TweakDB record ID in the TweakDB record
     * inheritance hierarchy if it exists, or an invalid TweakDBID otherwise.
     *
     * @param aRecordId The TweakDBID of the record to get the original parent record ID for.
     * @return The TweakDBID of the original parent record of the given TweakDB record ID in the TweakDB record
     * inheritance hierarchy if it exists, or an invalid TweakDBID otherwise.
     */
    [[nodiscard]] TweakDBID GetOriginalParent(TweakDBID aRecordId) const;

    /**
     * @brief Gets the set of TweakDBIDs of the original descendant records of the given TweakDB record ID in the
     * TweakDB record inheritance hierarchy if they exist, or an empty set otherwise.
     *
     * @param aSourceId The TweakDBID of the record to get the original descendant record IDs for.
     * @return The set of TweakDBIDs of the original descendant records of the given TweakDB record ID in the TweakDB
     * record inheritance hierarchy if they exist, or an empty set otherwise.
     */
    [[nodiscard]] const Core::Set<TweakDBID>& GetOriginalDescendants(TweakDBID aSourceId) const;

    /**
     * @brief Registers an extra flat for a TweakDB record type that is not defined by the original TweakDB record
     * class, but is added through registration. This allows for the addition of new properties to TweakDB record types
     * without modifying the original record classes, which can be useful for modding and extending the functionality of
     * TweakDB records.
     *
     * @param aRecordType The name of the TweakDB record type to register the extra flat for.
     * @param aPropName The name of the property corresponding to the extra flat to register.
     * @param aPropType The name of the RTTI type of the flat value of the property corresponding to the extra flat to
     * register.
     * @param aForeignType The name of the RTTI class type of the foreign record if the property corresponding to the
     * extra flat to register is a foreign key, or empty otherwise.
     */
    void RegisterExtraFlat(CName aRecordType, const std::string& aPropName, CName aPropType, CName aForeignType);

    /**
     * @brief Registers the relationships between an original TweakDB record and its descendant records in the TweakDB
     * record inheritance hierarchy.
     *
     * @param aParentId The TweakDBID of the original parent record to register the descendant relationships for.
     * @param aDescendantIds The set of TweakDBIDs of the original descendant records to register for the given original
     * parent record.
     */
    void RegisterDescendants(TweakDBID aParentId, const Core::Set<TweakDBID>& aDescendantIds);

    /**
     * @brief A utility function that converts a TweakDBID to its string representation, which can be useful for
     * debugging and logging purposes.
     *
     * @param aID The TweakDBID to convert to a string representation.
     * @return The string representation of the given TweakDBID.
     */
    [[nodiscard]] std::string ToString(TweakDBID aID) const;

    /**
     * @brief Gets the TweakDB instance used by this TweakDBReflection instance.
     *
     * @return The TweakDB instance used by this TweakDBReflection instance.
     */
    [[nodiscard]] TweakDB* GetTweakDB() const;

private:
    /**
     * @brief A struct representing the information of an extra flat that is not defined by the original TweakDB record
     * class, but is added through registration.
     */
    struct ExtraFlat
    {
        /**
         * @brief The name of the RTTI type of the flat value of the extra flat.
         */
        CName typeName;

        /**
         * @brief The name of the RTTI class type of the foreign record if the property corresponding to the extra flat
         * is a foreign key, or empty otherwise.
         */
        CName foreignTypeName;

        /**
         * @brief The appendix used to build the TweakDBID of the extra flat's value, which is appended to the record's
         * TweakDBID when retrieving the flat value from TweakDB.
         */
        std::string appendix;
    };

    /**
     * @brief A convenience type alias for the map of original TweakDB record IDs to their parent record IDs in the
     * TweakDB record inheritance hierarchy.
     */
    using ParentMap = Core::Map<TweakDBID, TweakDBID>;

    /**
     * @brief A convenience type alias for the map of original TweakDB record IDs to their descendant record IDs in the
     * TweakDB record inheritance hierarchy.
     */
    using DescendantMap = Core::Map<TweakDBID, Core::Set<TweakDBID>>;

    /**
     * @brief A convenience type alias for the map of TweakDB record type names to their corresponding extra flats.
     */
    using ExtraFlatMap = Core::Map<CName, Core::Vector<ExtraFlat>>;

    /**
     * @brief A convenience type alias for the map of TweakDB record type names to their corresponding reflection
     * information.
     */
    using RecordInfoMap = Core::Map<CName, Core::SharedPtr<TweakDBRecordInfo>>;

    /**
     * @brief Collects the reflection information of a TweakDB record type corresponding to the given RTTI class type by
     * inspecting the functions of the class type and extracting relevant details about the record type and its
     * properties.
     *
     * The collected reflection information is cached for future retrieval to optimize performance and avoid redundant
     * collection of the same record type information. This process is recursive and will crawl the type's parent tree
     * until all valid TweakDB record type details for the class' lineage have been discovered.
     *
     * @param aType The RTTI class type of the record type to collect the reflection information for.
     * @return A shared pointer to the reflection information of the TweakDB record type corresponding to the given RTTI
     * class type.
     */
    Core::SharedPtr<TweakDBRecordInfo> CollectRecordInfo(const CClass* aType);

    /**
     * @brief Resolves the name of the property corresponding to the given getter function name for a TweakDB record
     * type by inspecting the function name and extracting the property name based on typical naming conventions for
     * TweakDB record property getter functions.
     *
     * Only the direct getting function is considered (i.e. the function name that matches the property name). The name
     * may match the function name or have the first letter of the function name converted to lower case. The property's
     * TweakDB flat value is inspected to attempt to verify the correct name as part of this inference.
     *
     * @param aClass The RTTI class type of the TweakDB record type to resolve the property name for.
     * @param aGetterName The name of the getter function to resolve the property name for.
     * @return The resolved name of the property corresponding to the given getter function name for the TweakDB record
     * type.
     */
    std::string ResolvePropertyName(const CClass* aClass, CName aGetterName) const;

    /**
     * @brief Resolves the offset of the default value of a TweakDB record property in the TweakDB flat buffer
     * containing default values for the record type by inspecting the property's TweakDB flat value and determining its
     * position in the buffer.
     *
     * @param aType The RTTI class type of the TweakDB record type to resolve the default value offset for.
     * @param aPropName The name of the property to resolve the default value offset for.
     * @return The resolved offset of the default value of the property in the TweakDB flat buffer containing default
     * values for the record type, or -1 if the default value offset cannot be resolved.
     */
    int32_t ResolveDefaultValue(const CClass* aType, const std::string& aPropName) const;

    /**
     * @brief A pointer to the TweakDB instance used for this TweakDBReflection instance.
     */
    TweakDB* m_tweakDb;

    /**
     * @brief A pointer to the RTTI system, which is used for inspecting RTTI class types and functions.
     */
    CRTTISystem* m_rtti;

    /**
     * @brief A cache of reflection information for TweakDB record types, keyed by the record type name. This is used to
     * optimize performance by avoiding redundant collection of the same record type information through inspection of
     * RTTI class types and functions.
     */
    RecordInfoMap m_resolved;

    /**
     * @brief A mutex for synchronizing access to the cache of reflection information for TweakDB record types for
     * thread safety.
     */
    std::shared_mutex m_mutex;

    /**
     * @brief A mapping of descendant TweakDB record IDs to their parent record IDs. This is used for propagating
     * changes of properties in parent record instances to their descendants when they have inherited property values.
     */
    inline static ParentMap s_parentMap;

    /**
     * @brief A mapping of parent TweakDB record IDs to their descendant record IDs. This is used for propagating
     * changes of properties in parent record instances to their descendants when they have inherited property values.
     */
    inline static DescendantMap s_descendantMap;

    /**
     * @brief A cache of extra flats that are not defined by the original TweakDB record classes, but are added through
     * registration, keyed by the record type name.
     */
    inline static ExtraFlatMap s_extraFlats;
};
} // namespace Red
