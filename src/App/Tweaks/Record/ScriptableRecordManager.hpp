#pragma once

#include "App/Tweaks/Record/ScriptablePropertyManager.hpp"
#include "App/Tweaks/Record/ScriptableRecordClass.hpp"
#include "App/Tweaks/Record/ScriptableRecordTypes.hpp"
#include "Core/Logging/LoggingAgent.hpp"
#include "Red/ScriptBundle.hpp"
#include "Red/TweakDB/Manager.hpp"

namespace App
{
/**
 * @brief A manager for handling the registration and lifecycle of scriptable record types, their properties, and the
 * underlying components that allow them to function.
 */
class ScriptableRecordManager : public Core::LoggingAgent
{
public:
    /**
     * @brief Constructs a ScriptableRecordManager instance with the given TweakDB manager.
     *
     * @param aManager A deferred pointer to the TweakDB manager used to interact with TweakDB.
     * @param aPropertyHandler A shared pointer to the ScriptablePropertyHandler used to register script functions for
     * scriptable record properties and handle their invocation at runtime.
     */
    explicit ScriptableRecordManager(const Core::DeferredPtr<Red::TweakDBManager>& aManager,
                                     const Core::SharedPtr<ScriptablePropertyManager>& aPropertyHandler);

    /**
     * @brief Returns a vector containing the fully-qualified names of all registered scriptable record specifications.
     *
     * @return A vector containing the fully-qualified names of all registered scriptable record specifications.
     */
    std::vector<Red::CName> GetRecordSpecNames() const;

    /**
     * @brief Returns a vector containing shared pointers to all registered scriptable record specifications.
     *
     * @return A vector containing shared pointers to all registered scriptable record specifications.
     */
    Core::Vector<ScriptableRecordSpecPtr> GetRecordSpecs() const;

    /**
     * @brief Retrieves the scriptable record specification with the given name from the registry.
     *
     * @param aName The fully-qualified name of the record type corresponding to the desired specification.
     * @return A shared pointer to the scriptable record specification, or @c nullptr if no specification with the given
     * name was found.
     */
    ScriptableRecordSpecPtr GetRecordSpec(Red::CName aName) const;

    /**
     * @brief Retrieves the scriptable property specification with the given name from the registry for a specific
     * record type.
     *
     * @param aRecordName The fully-qualified name of the record type that the property belongs to.
     * @param aPropertyName The name of the property corresponding to the desired specification.
     * @return A shared pointer to the scriptable property specification, or @c nullptr if no specification with the
     * given name was found for the specified record type.
     */
    ScriptablePropertySpecPtr GetPropertySpec(Red::CName aRecordName, Red::CName aPropertyName) const;

    /**
     * @brief Attempts to construct a scriptable record instance and insert it into the given TweakDB instance. This
     * function is intended to be used as a means to directly inject scriptable record types into TweakDB and, in
     * particular, intercept normal Red::TweakDB::CreateRecord() invocations so that custom, scriptable record types may
     * function.
     *
     * If this function returns false, the normal record creation should follow to allow built-in TweakDB record types
     * to be built.
     *
     * @param aTweakDB Instance of TweakDB to insert records into.
     * @param aHash The hash of the record type to create. This should correspond to the hash of a registered scriptable
     * record type.
     * @param aRecordId The TweakDB ID of the record to create.
     * @return Whether the scriptable record was successfully created and inserted into the given TweakDB instance.
     */
    bool CreateScriptableRecord(Red::TweakDB* aTweakDB, uint32_t aHash, Red::TweakDBID aRecordId) const;

    /**
     * @brief Attempts to construct a scriptable record instance of the given class and insert it into the given TweakDB
     * instance. This function is intended to be used as a means to directly inject scriptable record types into TweakDB
     * and, in particular, intercept normal Red::TweakDB::CreateRecord() invocations so that custom, scriptable record
     * types may function.
     *
     * @param aTweakDB Instance of TweakDB to insert records into.
     * @param aClass The class of the record to create. This should correspond to a registered scriptable record class.
     * @param aRecordId The TweakDB ID of the record to create.
     * @return Whether the scriptable record was successfully created and inserted into the given TweakDB instance.
     */
    static bool CreateScriptableRecord(Red::TweakDB* aTweakDB, ScriptableRecordClass* aClass, Red::TweakDBID aRecordId);

    /**
     * @brief Registers a scriptable record type specification with this manager.
     *
     * The name of the scriptable record type will be normalized to adhere to the typical naming conventions of TweakDB
     * record types. Specifically, the type's fully-qualified name follows the format @c gamedata<record_name>_Record,
     * the script alias name follows the format @c <record_name>_Record, and the short name is the fully-qualified name
     * with the @c gamedata prefix and @c _Record suffix removed. For example, if a scriptable record type is registered
     * with the name @c Vehicle, the fully-qualified name will be @c gamedataVehicle_Record, the alias name will be
     * @c Vehicle_Record, and the short name will be @c Vehicle.
     *
     * If a valid TweakDB record type is provided as the superclass of the scriptable record type, the scriptable record
     * type will inherit all TweakDB properties of the superclass. If the class' parent name is not provided, it will
     * automatically be set to the base scriptable record type.
     *
     * @param aName The unique name of the scriptable record type to register.
     * @param aParentName The unique name of the parent scriptable record type to inherit from. This is optional and, if
     * not provided, will default to the base scriptable record type.
     * @return A shared pointer to the scriptable record's specification, or @c nullptr if registration failed for any
     * reason.
     */
    ScriptableRecordSpecPtr RegisterScriptableRecordType(const std::string& aName,
                                                         const std::optional<std::string>& aParentName = std::nullopt);

    /**
     * @brief Registers a scriptable property specification with a scriptable record type.
     *
     * @param aRecordSpec The specification of the scriptable record type that this property belongs to.
     * @param aPropertyName The name of the property to register.
     * @param aTypeSpec The type specification of the property, containing both its TweakDB flat type details and the
     * property type of the getter function.
     * @param aDefaultValue The default value of the property that will be inherited by instances of the record type
     * when no explicit value is provided for the instance.
     * @return A shared pointer to the registered property specification, or @c nullptr if registration failed for any
     * reason.
     */
    void RegisterScriptableProperty(const ScriptableRecordSpecPtr& aRecordSpec, const std::string& aPropertyName,
                                    const TweakTypeSpecPtr& aTypeSpec,
                                    const Red::InstancePtr<>& aDefaultValue = nullptr) const;

    /**
     * @brief Unregisters a scriptable record type and all of its properties from this manager based on the record's
     * name. This involves marking the record specification as deleted and unregistering all of its properties, which
     * includes marking their specifications as deleted and truncating the bytecode of  their corresponding property
     * handler functions.
     *
     * Note that this function does not remove the record type's corresponding RTTI class or any of its functions, so
     * the record type may still be present in RTTI after unregistration. However, the record type will be considered
     * invalid and should not be used after unregistration.
     *
     * @param aRecordName The fully-qualified name of the record type corresponding to the record specification to
     * unregister.
     */
    void UnregisterScriptableRecordType(Red::CName aRecordName) const;

    /**
     * @brief Unregisters a scriptable property from this manager based on the property's name and the record
     * specification of the record that it belongs to. This involves marking the property specification as deleted and
     * truncating the bytecode of its corresponding property handler functions.
     *
     * Note that this function does not remove any of the property's corresponding functions from RTTI, so the functions
     * may still be present in RTTI after unregistration. However, the property will be considered invalid and should
     * not be used after unregistration.
     *
     * @param aRecordSpec The specification of the record that this property belongs to.
     * @param aPropName The name of the property to unregister.
     */
    void UnregisterScriptableProperty(const ScriptableRecordSpecPtr& aRecordSpec, const std::string& aPropName) const;

    /**
     * @brief Updates a scriptable property with a new default value.
     *
     * @param aRecordSpec The specification of the record that this property belongs to.
     * @param aPropName The name of the property to update.
     * @param aDefaultValue The new default value of the property that will be inherited by instances of the record type
     * when no explicit value is provided for the instance.
     */
    void UpdateScriptableProperty(const ScriptableRecordSpecPtr& aRecordSpec, const std::string& aPropName,
                                  const Red::InstancePtr<>& aDefaultValue) const;
    /**
     * @brief Creates and registers RTTI classes for all pending scriptable record specifications registered with this
     * object. RTTI type registration only creates classes without any properties, functions, or inheritance.
     */
    void RegisterRTTITypes();

    /**
     * @brief Creates a @c ScriptableRecordClass and registers it with RTTI based on the provided record specification.
     * After completion, only the skeleton type will exist with its parent or any functions. If successful, the
     * specification may subsequently be described after all other scriptable records have been registered.
     *
     * @param aSpec The specification of the scriptable record type to create.
     * @return Whether the record class was successfully created and registered with RTTI.
     */
    bool RegisterRTTIType(const ScriptableRecordSpecPtr& aSpec);

    /**
     * @brief Completes setup of a scriptable record type by describing its corresponding RTTI class based on the
     * provided record specification. This involves setting the class's parent based on the parent specification of the
     * record type, if it exists.
     *
     * @param aSpec The specification of the scriptable record type to describe.
     * @return Whether the record class was successfully described.
     */
    bool DescribeRTTIType(const ScriptableRecordSpecPtr& aSpec) const;

    /**
     * @brief Creates and registers RTTI functions for all properties of the given record specification based on their
     * property specifications. After completion, the functions will be fully set up to invoke the appropriate property
     * getters at runtime.
     *
     * @param aRecordSpec The specification of the scriptable record type for which to create property functions.
     * @param aPropSpec The specification of the property for which to create property functions.
     * @return Whether all functions for the properties of the given record specification were successfully created and
     * registered with RTTI.
     */
    void CreatePropertyFunctions(const ScriptableRecordSpecPtr& aRecordSpec,
                                 const ScriptablePropertySpecPtr& aPropSpec) const;

    /**
     * @brief Inserts default values for all scriptable records specifications registered with this object into the
     * provided TweakDB manager. This should only be called after all specs have been registered and described.
     *
     * In particular, a TweakDB flat instance will be inserted into TweakDB with the default value for any direct
     * property of the record with the TweakDB ID
     * @c RTDB.<record_name>.<property_name>, where @c property_name is the name of the property.
     */
    void InsertDefaultValues();

    /**
     * @brief Inspects the provided script bundle for any script classes corresponding to registered scriptable record
     * types and, if any are found, modifies the bytecode of their functions to invoke the appropriate property handlers
     * at runtime.
     *
     * @param aClasses The array of script classes parsed from a script bundle to inspect for functions corresponding to
     * registered scriptable record types and modify to invoke property handlers at runtime.
     */
    void AdaptScriptClasses(const Red::DynArray<Red::ScriptClass*>& aClasses) const;

    /**
     * @brief Returns whether the record manager has been notified that TweakDB is ready for insertion of default values
     * for scriptable record types. This is used to gate the insertion of default values for scriptable record types
     * until TweakDB is ready to ensure that the insertion process completes successfully.
     *
     * @return Whether the record manager has been notified that TweakDB is ready for insertion of default values for
     * scriptable record types.
     */
    bool IsTweakDBReady() const;

    /**
     * @brief Notifies the record manager that TweakDB is ready for insertion of default values for scriptable record
     * types. This should be called in the TweakDB post-registration callback to ensure that insertion of default values
     * for scriptable record types is properly gated until TweakDB is ready.
     *
     * @param aTweakDBReady Whether TweakDB is ready for insertion of default values for scriptable record types.
     */
    void SetTweakDBReady(bool aTweakDBReady = true);

#ifndef NDEBUG
    /**
     * @brief A debug-enabled helper function for orchestrating the complete registration process of a scriptable record
     * specification for use in running test cases.
     *
     * @param aSpec The scriptable record specification to fully register, including RTTI registration, RTTI
     * description, and default value insertion into TweakDB.
     *
     * @return Whether the complete registration process for the given scriptable record specification was successful.
     * If this function returns false, the specification should not be used for testing as it may be in an incomplete or
     * invalid state.
     */
    bool SetupTestRecord(const ScriptableRecordSpecPtr& aSpec);
#endif

private:
    /**
     * @brief Unregisters a scriptable property from this manager based on the property's specification and the record
     * specification of the record that it belongs to. This involves marking the property specification as deleted and
     * truncating the bytecode of its corresponding property handler functions.
     *
     * @param aRecordSpec The specification of the record that this property belongs to.
     * @param aPropSpec The specification of the property to unregister.
     */
    void UnregisterScriptableProperty(const ScriptableRecordSpecPtr& aRecordSpec,
                                      const ScriptablePropertySpecPtr& aPropSpec) const;

    /**
     * @brief Inserts default values for a scriptable record type based on the provided record specification.
     *
     * This involves inserting default values for each direct property of the record with the TweakDB ID
     * @c RTDB.<record_name>.<property_name>, where @c property_name is the name of the property.
     *
     * @param aSpec The specification of the scriptable record type for which to insert default values. This should be a
     * valid record specification contained in the registry.
     */
    void InsertDefaultValues(const ScriptableRecordSpecPtr& aSpec);

    /**
     * @brief Inserts default values for a scriptable record type based on the provided RTTI class. This is an overload
     * of the previous function that retrieves the record specification based on the class's name and then calls the
     * previous function to perform the insertion.
     *
     * @param aClass The RTTI class of the record type for which to insert default values. This should correspond to the
     * class of a registered scriptable record type.
     */
    void InsertDefaultValues(const Red::CClass* aClass);

    /**
     * @brief Retrieves the scriptable record class corresponding to the given hash from the registry.
     *
     * @param aHash The hash of the short name of the record class to retrieve.
     * @return A pointer to the scriptable record class, or @c nullptr if no class with the given hash was found.
     */
    ScriptableRecordClass* GetRecordClass(uint32_t aHash) const;

    /**
     * @brief Creates a scriptable record class based on the provided record specification and registers it with RTTI.
     *
     * @param aSpec The specification of the scriptable record type for which to create a scriptable record class.
     * @return A pointer to the created scriptable record class, or @c nullptr if the class could not be created for any
     * reason.
     */
    ScriptableRecordClass* CreateRecordClass(const ScriptableRecordSpecPtr& aSpec);

    /**
     * @brief Inspects the provided script class for any functions corresponding to the registered scriptable record
     * type with a name matching the class's name. For any that are found, their bytecode will be modified to invoke the
     * appropriate property handlers at runtime.
     *
     * @param aClassDef The script class parsed from RedScript to inspect for functions corresponding to the registered
     * scriptable record type with a name matching the class's name and modify to invoke property handlers at runtime.
     */
    void AdaptScriptClass(const Red::ScriptClass* aClassDef) const;

    /**
     * @brief A pointer to the RTTI system, which is used for registering scriptable record types as RTTI classes. This
     * is initialized in the constructor and should always be valid for the lifetime of this object.
     */
    Red::CRTTISystem* m_rtti;

    /**
     * @brief A mutex for synchronizing access to the scriptable record specification registry for thread safety.
     */
    mutable std::shared_mutex m_specsMutex;

    /**
     * @brief A registry of scriptable record specifications, indexed by the CName of each record type's fully-qualified
     * name.
     */
    Core::Map<Red::CName, ScriptableRecordSpecPtr> m_specs;

    /**
     * @brief A mutex for synchronizing access to the scriptable record class registry for thread safety.
     */
    mutable std::shared_mutex m_classesMutex;

    /**
     * @brief A registry of scriptable record classes, indexed by the hash of each record type's short name.
     */
    Core::Map<uint32_t, ScriptableRecordClass*> m_classes;

    /**
     * @brief A pointer to the TweakDB manager.
     */
    Core::DeferredPtr<Red::TweakDBManager> m_tweakManager;

    /**
     * @brief A shared pointer to the manager for scriptable property getters.
     */
    Core::SharedPtr<ScriptablePropertyManager> m_propertyHandler;

    /**
     * @brief A flag indicating whether the TweakDB manager is ready for inserting default values for scriptable record
     * types. This is set in a post-register callback after TweakDB initialization and is used to ensure that default
     * values for scriptable record types are not inserted before TweakDB is ready, which would cause issues with the
     * insertion process.
     */
    bool m_tweakDBReady = false;
};
} // namespace App
