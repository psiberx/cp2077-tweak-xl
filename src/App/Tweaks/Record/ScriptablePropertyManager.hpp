#pragma once

#include "Core/Logging/LoggingAgent.hpp"
#include "ScriptableRecordTypes.hpp"

namespace App
{
/**
 * @brief A concept to constrain the type of handle used in record array getters to either Red::Handle or
 * Red::WeakHandle, both instantiated with Red::TweakDBRecord as the template argument.
 */
template<template<typename> typename THandle>
concept IsHandleType = std::is_same_v<THandle<Red::TweakDBRecord>, Red::Handle<Red::TweakDBRecord>> ||
                       std::is_same_v<THandle<Red::TweakDBRecord>, Red::WeakHandle<Red::TweakDBRecord>>;

/**
 * @brief An enumeration of the different types of property getter functions that can be registered for scriptable
 * TweakDB records, used to identify the appropriate function getter implementation for a given function based on its
 * name. The function getters corresponding to each getter type are implemented in App::ScriptablePropertyGetter and its
 * derived classes.
 */
enum class GetterType : uint8_t
{
    GetRecordArray,
    RecordArrayContains,
    GetRecordItem,
    GetRecordItemHandle,
    GetRecord,
    GetRecordHandle,
    GetArrayCount,
    GetArrayItem,
    ArrayContains,
    GetResRefArray,
    GetResRefItem,
    GetResRef,
    GetLocKeyArray,
    GetLocKeyItem,
    LocKeyArrayContains,
    GetLocKey,
    Get
};

/**
 * @brief A struct representing the execution context for a script function invocation corresponding to a scriptable
 * record property getter.
 *
 * This context contains the property specification associated with the invoked function, as well as pointers to any
 * services required to handle the function's invocation, such as the TweakDB manager. This context is intended to be
 * created at the time of script function registration for a given property specification and stored for use during
 * function invocation at runtime.
 */
struct Context
{
    /**
     * @brief The path to append to a scriptable record's TweakDB ID containing the property's value.
     */
    std::string appendix;

    /**
     * @brief The type specification of the property associated with the invoked function, containing both its TweakDB
     * flat type details and the property type of the getter function.
     */
    TweakTypeSpecPtr typeSpec;

    /**
     * @brief A deferred shared pointer to the TweakDB manager.
     */
    Core::DeferredPtr<Red::TweakDBManager> tweakManager;
};

/**
 * @brief A base class for handling the processing of script functions that serve as property getters for scriptable
 * TweakDB records. Each derived class corresponds to a specific type of getter function, identified by the GetterType
 * enumeration, and implements the necessary logic for:
 *
 * - Handling the function's invocation at runtime.
 * - Constructing a function hash based on the expected signature of the getter function.
 * - Defining the expected signature of the getter function.
 *
 * This class (and its derived classes) can only be created at compile time.
 */
class ScriptablePropertyGetter
{
public:
    /**
     * @brief Constructs a ScriptablePropertyGetter with the given type, prefix, and suffix.
     *
     * The prefix and suffix are used to generate the names of the script functions that this getter will manage, which
     * follow the format @c <prefix><property_name><suffix>. For example, if the prefix is @c Get and the suffix is @c
     * Item, a property named @c MyProperty would correspond to a function named @c GetMyPropertyItem.
     *
     * @param aPrefix The prefix added to the names of generated script functions for this getter.
     * @param aSuffix The suffix added to the names of generated script functions for this getter.
     */
    consteval explicit ScriptablePropertyGetter(const std::string_view aPrefix, const std::string_view aSuffix)
        : m_prefix(aPrefix)
        , m_suffix(aSuffix)
        , m_prefixLength(aPrefix.length())
        , m_suffixLength(aSuffix.length())
    {
    }

    /**
     * @brief Virtual destructor for ScriptablePropertyGetter.
     */
    virtual ~ScriptablePropertyGetter() = default;

    /**
     * @brief Handles the invocation of a script function corresponding to this property getter's specialization at
     * runtime, performing the necessary logic to retrieve the relevant property value from TweakDB based on the given
     * execution context.
     *
     * @param aInstance The scriptable record instance on which the function is being invoked.
     * @param aFrame The stack frame for the function invocation, which can be used to access the function's arguments.
     * @param aOut A pointer to the memory location where the function's return value should be stored, if applicable.
     * @param aContext The execution context containing the property specification and pointers to required services.
     */
    virtual void HandleInvocation(Red::IScriptable* aInstance, Red::CStackFrame* aFrame, void* aOut,
                                  const Context* aContext) const = 0;

    /**
     * @brief Generates the hash of the function name corresponding to the given record and property specifications for
     * this property getter. This hash is used to establish a link between a function parsed from RedScript to the
     * property specification associated with it.
     *
     * The function name is generated based on the naming convention defined by this getter's prefix and suffix, as well
     * as the function name specified in the property specification. For example, if the prefix is @c Get and the suffix
     * is @c Item, a property with the function name @c MyProperty would correspond to a function named @c
     * GetMyPropertyItem.
     *
     * The hash is computed from a semi-color delimited string of various string segments. The segment order is as
     * follows:
     *
     * - Record class name.
     * - Function return type, or "void" if it does not have a return.
     * - Generated function name for the getter.
     * - Types for all function arguments, sequentially, if the function has any.
     *
     * @param aRecordSpec The specification of the scriptable record type that this function belongs to.
     * @param aPropSpec The specification of the property that this function serves as a getter for.
     * @return The hash of the function name corresponding to the given record and property specifications for this
     * property getter.
     */
    [[nodiscard]] virtual Red::CName GetFunctionHash(const ScriptableRecordSpecPtr& aRecordSpec,
                                                     const ScriptablePropertySpecPtr& aPropSpec) const = 0;

    /**
     * @brief Extracts the base property name from a given function name by removing the getter's prefix and suffix.
     * For example, if the getter's prefix is @c Get and suffix is @c Item, a function name @c GetMyPropertyItem would
     * yield a base property name of @c MyProperty.
     *
     * @param aName The function name to extract the base property name from.
     * @return The base property name extracted from the given function name, or an empty string if the function name
     * does not conform to the expected format with this getter's prefix and suffix.
     */
    [[nodiscard]] std::string GetFunctionBaseName(const std::string& aName) const;

    /**
     * @brief Generates the full function name for a given base property name by adding this getter's prefix and suffix.
     * For example, if the getter's prefix is @c Get and suffix is @c Item, a base property name of @c MyProperty would
     * yield a function name of @c GetMyPropertyItem.
     *
     * @param aName The base property name to generate the function name for.
     * @return The full function name generated by adding this getter's prefix and suffix to the given base property
     * name.
     */
    [[nodiscard]] std::string GetFunctionName(const std::string& aName) const;

    /**
     * @brief Configures the given script function's arguments and return type based on the specialized requirements of
     * the getter function and the provided property specification.
     *
     * @param aFunction The script function to configure with the appropriate arguments and return type.
     * @param aPropSpec The specification of the property that this function serves as a getter for.
     */
    virtual void ConfigureScriptFunction(Red::CClassFunction* aFunction,
                                         const ScriptablePropertySpecPtr& aPropSpec) const = 0;

protected:
    /**
     * @brief Retrieves the TweakDB ID of a property's flat value by appending the property's flat appendix to the given
     * scriptable record instance's TweakDB ID.
     *
     * @param aInstance The scriptable record instance for which to retrieve the record's ID.
     * @param aContext The execution context containing the property specification from which to retrieve the flat ID
     * appendix.
     * @return The TweakDB ID of the property's flat value, which is used to retrieve the flat instance from TweakDB at
     * runtime.
     */
    Red::TweakDBID GetFlatID(Red::Instance aInstance, const Context* aContext) const;

    /**
     * @brief Retrieves an array of record handles from a TweakDB flat value corresponding to a property, if the flat
     * value is valid and of the expected type. This is used by various property getter functions to retrieve the array
     * of records associated with a property from TweakDB at runtime.
     *
     * @tparam THandle The type of handle to use for the records in the array, which can be either Red::Handle or
     * Red::WeakHandle.
     * @param aValue The TweakDB flat value from which to retrieve the array of record handles. This is expected to be
     * an array of TweakDB IDs.
     * @param aContext The execution context containing the property specification from which to retrieve the expected
     * type of the records in the array.
     * @return An optional containing the array of record handles retrieved from the given TweakDB flat value if it is
     * valid and of the expected type, or an empty optional if the value is invalid or not of the expected type.
     */
    template<template<typename> typename THandle>
        requires IsHandleType<THandle>
    std::optional<Red::DynArray<THandle<Red::TweakDBRecord>>> GetRecordArray(const Red::Value<>& aValue,
                                                                             const Context* aContext) const;
    /**
     * @brief A prefix added to the names of generated script functions for this property getter based on the name of
     * the property.
     *
     * For example, if the prefix is @c Get and the property name is @c MyProperty, the generated function
     * name would be updated to @c GetMyProperty.
     */
    const std::string_view m_prefix;

    /**
     * @brief A suffix added to the names of generated script functions for this property getter based on the name of
     * the property.
     *
     * For example, if the suffix is @c Item and the property name is @c MyProperty, the generated function name would
     * be updated to @c GetMyPropertyItem.
     */
    const std::string_view m_suffix;

    /**
     * @brief The length of the prefix string, cached for efficiency in extracting the base property name from function
     * names.
     */
    const size_t m_prefixLength;

    /**
     * @brief The length of the suffix string, cached for efficiency in extracting the base property name from function
     * names.
     */
    const size_t m_suffixLength;
};

/**
 * @brief A template class for handling the registration and invocation of script functions that serve as property
 * getters for scriptable TweakDB records, providing the ability to create specialized getters per getter type.
 */
template<GetterType>
class TTypedPropertyGetter : public ScriptablePropertyGetter
{
public:
    /**
     * @brief Constructs a TTypedPropertyGetter with no name prefix or suffix.
     */
    consteval TTypedPropertyGetter()
        : ScriptablePropertyGetter("", "")
    {
    }

    /**
     * @brief Constructs a TTypedPropertyGetter with the given name suffix and no prefix.
     *
     * @param aSuffix The suffix added to the names of generated script functions for this getter.
     */
    consteval explicit TTypedPropertyGetter(const std::string_view aSuffix)
        : ScriptablePropertyGetter("", aSuffix)
    {
    }

    /**
     * @brief Constructs a TTypedPropertyGetter with the given name prefix and no suffix.
     *
     * @param aPrefix The prefix added to the names of generated script functions for this getter.
     * @param aSuffix The suffix added to the names of generated script functions for this getter.
     */
    consteval explicit TTypedPropertyGetter(const std::string_view aPrefix, const std::string_view aSuffix)
        : ScriptablePropertyGetter(aPrefix, aSuffix)
    {
    }
};

/**
 * @brief A property getter that retrieves an array of foreign keys to TweakDB records from a scriptable record
 * property.
 *
 * The generated function name for this getter has no additional prefix or suffix beyond the base property name. For
 * example, a property named @c MyProperty would correspond to a function named @c MyProperty.
 *
 * The TweakDB flat associated with the property is expected to contain an array of TweakDB IDs that point to TweakDB
 * records of a specific type, as defined by the property. During processing, the results are validated to ensure that
 * target records conform to the expected type. On success, the getter will return an array of weak handles to the
 * target TweakDB records.
 */
class RecordArrayGetter : public TTypedPropertyGetter<GetterType::GetRecordArray>
{
public:
    consteval RecordArrayGetter() = default;
    [[nodiscard]] Red::CName GetFunctionHash(const ScriptableRecordSpecPtr& aRecordSpec,
                                             const ScriptablePropertySpecPtr& aPropSpec) const override;
    void HandleInvocation(Red::IScriptable* aInstance, Red::CStackFrame* aFrame, void* aOut,
                          const Context* aContext) const override;
    void ConfigureScriptFunction(Red::CClassFunction* aFunction,
                                 const ScriptablePropertySpecPtr& aPropSpec) const override;
};

/**
 * @brief A property getter that checks whether a TweakDB record is contained within an array of foreign keys to TweakDB
 * records associated with a scriptable record property.
 *
 * The generated function name for this getter has the suffix @c Contains and no prefix. For example, a property named
 * @c MyProperty would correspond to a function named @c MyPropertyContains.
 *
 * The TweakDB flat associated with the property is expected to contain an array of TweakDB IDs that point to TweakDB
 * records of a specific type, as defined by the property. During processing, the results are validated to ensure that
 * target records conform to the expected type. The getter takes a weak handle to a TweakDB record as an argument and
 * checks whether it is contained within the array of records associated with the property, returning true if it is and
 * false otherwise.
 */
class RecordArrayContainsGetter : public TTypedPropertyGetter<GetterType::RecordArrayContains>
{
public:
    consteval RecordArrayContainsGetter()
        : TTypedPropertyGetter("Contains")
    {
    }

    [[nodiscard]] Red::CName GetFunctionHash(const ScriptableRecordSpecPtr& aRecordSpec,
                                             const ScriptablePropertySpecPtr& aPropSpec) const override;
    void HandleInvocation(Red::IScriptable* aInstance, Red::CStackFrame* aFrame, void* aOut,
                          const Context* aContext) const override;
    void ConfigureScriptFunction(Red::CClassFunction* aFunction,
                                 const ScriptablePropertySpecPtr& aPropSpec) const override;
};

/**
 * @brief A property getter that returns a foreign key to a TweakDB record from an array of TweakDB records from
 * scriptable record property.
 *
 * The generated function name for this getter has the prefix @c Get and the suffix @c Item. For example, a property
 * named @c MyProperty would correspond to a function named @c GetMyPropertyItem.
 *
 * The TweakDB flat associated with the property is expected to contain an array of TweakDB IDs that point to TweakDB
 * records of a specific type, as defined by the property. During processing, the results are validated to ensure that
 * target records conform to the expected type. The getter takes an index as an argument and returns a weak handle to
 * the TweakDB record at that index within the array of records associated with the property.
 */
class RecordItemGetter : public TTypedPropertyGetter<GetterType::GetRecordItem>
{
public:
    consteval RecordItemGetter()
        : TTypedPropertyGetter("Get", "Item")
    {
    }

    [[nodiscard]] Red::CName GetFunctionHash(const ScriptableRecordSpecPtr& aRecordSpec,
                                             const ScriptablePropertySpecPtr& aPropSpec) const override;
    void HandleInvocation(Red::IScriptable* aInstance, Red::CStackFrame* aFrame, void* aOut,
                          const Context* aContext) const override;
    void ConfigureScriptFunction(Red::CClassFunction* aFunction,
                                 const ScriptablePropertySpecPtr& aPropSpec) const override;
};

/**
 * @brief A property getter that returns a foreign key to a TweakDB record from an array of TweakDB records from
 * scriptable record property, where the returned handle is a strong handle rather than a weak handle.
 *
 * The generated function name for this getter has the prefix @c Get and the suffix @c ItemHandle. For example, a
 * property named @c MyProperty would correspond to a function named @c GetMyPropertyItemHandle.
 *
 * The TweakDB flat associated with the property is expected to contain an array of TweakDB IDs that point to TweakDB
 * records of a specific type, as defined by the property. During processing, the results are validated to ensure that
 * target records conform to the expected type. The getter takes an index as an argument and returns a strong handle to
 * the TweakDB record at that index within the array of records associated with the property.
 */
class RecordItemHandleGetter : public TTypedPropertyGetter<GetterType::GetRecordItemHandle>
{
public:
    consteval RecordItemHandleGetter()
        : TTypedPropertyGetter("Get", "ItemHandle")
    {
    }

    [[nodiscard]] Red::CName GetFunctionHash(const ScriptableRecordSpecPtr& aRecordSpec,
                                             const ScriptablePropertySpecPtr& aPropSpec) const override;
    void HandleInvocation(Red::IScriptable* aInstance, Red::CStackFrame* aFrame, void* aOut,
                          const Context* aContext) const override;
    void ConfigureScriptFunction(Red::CClassFunction* aFunction,
                                 const ScriptablePropertySpecPtr& aPropSpec) const override;
};

/**
 * @brief A property getter that retrieves a foreign key to a TweakDB record from a scriptable record property.
 *
 * The generated function name for this ter has no additional prefix or suffix beyond the base property name. For
 * example, a property named @c MyProperty would correspond to a function named @c MyProperty.
 *
 * The TweakDB flat associated with the property is expected to contain a TweakDB ID that points to a TweakDB record of
 * a specific type, as defined by the property. During processing, the result is validated to ensure that the target
 * record conforms to the expected type. On success, the getter will return a weak handle to the target TweakDB record.
 */
class RecordGetter : public TTypedPropertyGetter<GetterType::GetRecord>
{
public:
    consteval RecordGetter() = default;

    [[nodiscard]] Red::CName GetFunctionHash(const ScriptableRecordSpecPtr& aRecordSpec,
                                             const ScriptablePropertySpecPtr& aPropSpec) const override;
    void HandleInvocation(Red::IScriptable* aInstance, Red::CStackFrame* aFrame, void* aOut,
                          const Context* aContext) const override;
    void ConfigureScriptFunction(Red::CClassFunction* aFunction,
                                 const ScriptablePropertySpecPtr& aPropSpec) const override;
};

/**
 * @brief A property getter that retrieves a foreign key to a TweakDB record from a scriptable record property, where
 * the returned handle is a strong handle rather than a weak handle.
 *
 * The generated function name for this getter has the suffix @c Handle and no prefix. For example, a property named
 * @c MyProperty would correspond to a function named @c MyPropertyHandle.
 *
 * The TweakDB flat associated with the property is expected to contain a TweakDB ID that points to a TweakDB record of
 * a specific type, as defined by the property. During processing, the result is validated to ensure that the target
 * record conforms to the expected type. On success, the getter will return a strong handle to the target TweakDB
 * record.
 */
class RecordHandleGetter : public TTypedPropertyGetter<GetterType::GetRecordHandle>
{
public:
    consteval RecordHandleGetter()
        : TTypedPropertyGetter("Handle")
    {
    }

    [[nodiscard]] Red::CName GetFunctionHash(const ScriptableRecordSpecPtr& aRecordSpec,
                                             const ScriptablePropertySpecPtr& aPropSpec) const override;
    void HandleInvocation(Red::IScriptable* aInstance, Red::CStackFrame* aFrame, void* aOut,
                          const Context* aContext) const override;
    void ConfigureScriptFunction(Red::CClassFunction* aFunction,
                                 const ScriptablePropertySpecPtr& aPropSpec) const override;
};

/**
 * @brief A property getter that retrieves the count of items in an array associated with a scriptable record property.
 *
 * The generated function name for this getter has the prefix @c Get and the suffix @c Count. For example, a property
 * named @c MyProperty would correspond to a function named @c GetMyPropertyCount.
 *
 * The TweakDB flat associated with the property is expected to contain an array of any type. During processing, the
 * result is validated to ensure that it is an array of the expected type. On success, the getter will return the count
 * of items in the array.
 */
class ArrayCountGetter : public TTypedPropertyGetter<GetterType::GetArrayCount>
{
public:
    consteval ArrayCountGetter()
        : TTypedPropertyGetter("Get", "Count")
    {
    }

    [[nodiscard]] Red::CName GetFunctionHash(const ScriptableRecordSpecPtr& aRecordSpec,
                                             const ScriptablePropertySpecPtr& aPropSpec) const override;
    void HandleInvocation(Red::IScriptable* aInstance, Red::CStackFrame* aFrame, void* aOut,
                          const Context* aContext) const override;
    void ConfigureScriptFunction(Red::CClassFunction* aFunction,
                                 const ScriptablePropertySpecPtr& aPropSpec) const override;
};

/**
 * @brief A property getter that retrieves an item at a specific index from an array associated with a scriptable record
 * property.
 *
 * The generated function name for this getter has the prefix @c Get and the suffix @c Item. For example, a property
 * named @c MyProperty would correspond to a function named @c GetMyPropertyItem.
 *
 * The TweakDB flat associated with the property is expected to contain an array of any type other than foreign keys to
 * other TweakDB records, resource references, or localization keys. During processing, the result is validated to
 * ensure that it is an array and that its elements are of the expected type as defined by the property specification.
 * On success, the getter will return the item at the specified index in the array.
 */
class ArrayItemGetter : public TTypedPropertyGetter<GetterType::GetArrayItem>
{
public:
    consteval ArrayItemGetter()
        : TTypedPropertyGetter("Get", "Item")
    {
    }

    [[nodiscard]] Red::CName GetFunctionHash(const ScriptableRecordSpecPtr& aRecordSpec,
                                             const ScriptablePropertySpecPtr& aPropSpec) const override;
    void HandleInvocation(Red::IScriptable* aInstance, Red::CStackFrame* aFrame, void* aOut,
                          const Context* aContext) const override;
    void ConfigureScriptFunction(Red::CClassFunction* aFunction,
                                 const ScriptablePropertySpecPtr& aPropSpec) const override;
};

/**
 * @brief A property getter that checks whether a specific item is contained within an array associated with a
 * scriptable record property.
 *
 * The generated function name for this getter has the suffix @c Contains and the prefix @c Get. For example, a
 * property named @c MyProperty would correspond to a function named @c GetMyPropertyContains.
 *
 * The TweakDB flat associated with the property is expected to contain an array of any type other than foreign keys to
 * other TweakDB records, resource references, or localization keys. During processing, the result is validated to
 * ensure that it is an array and that its elements are of the expected type as defined by the property specification.
 * The getter takes an item as an argument and checks whether it is contained within the array associated with the
 * property, returning true if it is and false otherwise.
 */
class ArrayContainsGetter : public TTypedPropertyGetter<GetterType::ArrayContains>
{
public:
    consteval ArrayContainsGetter()
        : TTypedPropertyGetter("Contains")
    {
    }

    [[nodiscard]] Red::CName GetFunctionHash(const ScriptableRecordSpecPtr& aRecordSpec,
                                             const ScriptablePropertySpecPtr& aPropSpec) const override;
    void HandleInvocation(Red::IScriptable* aInstance, Red::CStackFrame* aFrame, void* aOut,
                          const Context* aContext) const override;
    void ConfigureScriptFunction(Red::CClassFunction* aFunction,
                                 const ScriptablePropertySpecPtr& aPropSpec) const override;
};

/**
 * @brief A property getter that retrieves an array of resource references from a scriptable record property.
 *
 * The generated function name for this getter has no additional prefix or suffix beyond the base property name. For
 * example, a property named @c MyProperty would correspond to a function named @c MyProperty.
 *
 * The TweakDB flat associated with the property is expected to contain an array of @c RaRef<CResource>. During
 * processing, the result is validated to ensure that it is an array and that its elements are of the expected type. On
 * success, the getter will return the array of @c ResRef .
 */
class ResRefArrayGetter : public TTypedPropertyGetter<GetterType::GetResRefArray>
{
public:
    consteval ResRefArrayGetter() = default;
    [[nodiscard]] Red::CName GetFunctionHash(const ScriptableRecordSpecPtr& aRecordSpec,
                                             const ScriptablePropertySpecPtr& aPropSpec) const override;
    void HandleInvocation(Red::IScriptable* aInstance, Red::CStackFrame* aFrame, void* aOut,
                          const Context* aContext) const override;
    void ConfigureScriptFunction(Red::CClassFunction* aFunction,
                                 const ScriptablePropertySpecPtr& aPropSpec) const override;
};

/**
 * @brief A property getter that retrieves an item at a specific index from an array of resource references associated
 * with a scriptable record property.
 *
 * The generated function name for this getter has the prefix @c Get and the suffix @c Item. For example, a property
 * named @c MyProperty would correspond to a function named @c GetMyPropertyItem.
 *
 * The TweakDB flat associated with the property is expected to contain an array of @c RaRef<CResource>. During
 * processing, the result is validated to ensure that it is of the expected type. On success, the getter will return
 * the @c ResRef at the specified index in the array.
 */
class ResRefItemGetter : public TTypedPropertyGetter<GetterType::GetResRefItem>
{
public:
    consteval ResRefItemGetter()
        : TTypedPropertyGetter("Get", "Item")
    {
    }

    [[nodiscard]] Red::CName GetFunctionHash(const ScriptableRecordSpecPtr& aRecordSpec,
                                             const ScriptablePropertySpecPtr& aPropSpec) const override;
    void HandleInvocation(Red::IScriptable* aInstance, Red::CStackFrame* aFrame, void* aOut,
                          const Context* aContext) const override;
    void ConfigureScriptFunction(Red::CClassFunction* aFunction,
                                 const ScriptablePropertySpecPtr& aPropSpec) const override;
};

/**
 * @brief A property getter that retrieves a resource reference from a scriptable record property.
 *
 * The generated function name for this getter has no additional prefix or suffix beyond the base property name. For
 * example, a property named @c MyProperty would correspond to a function named @c MyProperty.
 *
 * The TweakDB flat associated with the property is expected to contain a @c RaRef<CResource>. During processing, the
 * result is validated to ensure that it is of the expected type. On success, the getter will return the @c ResRef .
 */
class ResRefGetter : public TTypedPropertyGetter<GetterType::GetResRef>
{
public:
    consteval ResRefGetter() = default;
    [[nodiscard]] Red::CName GetFunctionHash(const ScriptableRecordSpecPtr& aRecordSpec,
                                             const ScriptablePropertySpecPtr& aPropSpec) const override;
    void HandleInvocation(Red::IScriptable* aInstance, Red::CStackFrame* aFrame, void* aOut,
                          const Context* aContext) const override;
    void ConfigureScriptFunction(Red::CClassFunction* aFunction,
                                 const ScriptablePropertySpecPtr& aPropSpec) const override;
};

/**
 * @brief A property getter that retrieves a localization key from a scriptable record property.
 *
 * The generated function name for this getter has no additional prefix or suffix beyond the base property name. For
 * example, a property named @c MyProperty would correspond to a function named @c MyProperty.
 *
 * The TweakDB flat associated with the property is expected to contain a @c LocKey. During processing, the result is
 * validated to ensure that it is of the expected type. On success, the getter will return the value as a @c CName .
 */
class LocKeyGetter : public TTypedPropertyGetter<GetterType::GetLocKey>
{
public:
    consteval LocKeyGetter() = default;
    [[nodiscard]] Red::CName GetFunctionHash(const ScriptableRecordSpecPtr& aRecordSpec,
                                             const ScriptablePropertySpecPtr& aPropSpec) const override;
    void HandleInvocation(Red::IScriptable* aInstance, Red::CStackFrame* aFrame, void* aOut,
                          const Context* aContext) const override;
    void ConfigureScriptFunction(Red::CClassFunction* aFunction,
                                 const ScriptablePropertySpecPtr& aPropSpec) const override;
};

/**
 * @brief A property getter that retrieves an array of localization keys from a scriptable record property.
 *
 * The generated function name for this getter has no additional prefix or suffix beyond the base property name. For
 * example, a property named @c MyProperty would correspond to a function named @c MyProperty.
 *
 * The TweakDB flat associated with the property is expected to contain an array of @c LocKey. During processing, the
 * result is validated to ensure that it is of the expected type. On success, the getter will return the value as an
 * array of @c CName .
 */
class LocKeyArrayGetter : public TTypedPropertyGetter<GetterType::GetLocKeyArray>
{
public:
    consteval LocKeyArrayGetter() = default;
    [[nodiscard]] Red::CName GetFunctionHash(const ScriptableRecordSpecPtr& aRecordSpec,
                                             const ScriptablePropertySpecPtr& aPropSpec) const override;
    void HandleInvocation(Red::IScriptable* aInstance, Red::CStackFrame* aFrame, void* aOut,
                          const Context* aContext) const override;
    void ConfigureScriptFunction(Red::CClassFunction* aFunction,
                                 const ScriptablePropertySpecPtr& aPropSpec) const override;
};

/**
 * @brief A property getter that retrieves an item at a specific index from an array of localization keys associated
 * with a scriptable record property.
 *
 * The generated function name for this getter has the prefix @c Get and the suffix @c Item. For example, a property
 * named @c MyProperty would correspond to a function named @c GetMyPropertyItem.
 *
 * The TweakDB flat associated with the property is expected to contain an array of @c LocKey. During processing, the
 * result is validated to ensure that it is of the expected type. On success, the getter will return the @c CName at the
 * specified index in the array.
 */
class LocKeyItemGetter : public TTypedPropertyGetter<GetterType::GetLocKeyItem>
{
public:
    consteval LocKeyItemGetter()
        : TTypedPropertyGetter("Get", "Item")
    {
    }

    [[nodiscard]] Red::CName GetFunctionHash(const ScriptableRecordSpecPtr& aRecordSpec,
                                             const ScriptablePropertySpecPtr& aPropSpec) const override;
    void HandleInvocation(Red::IScriptable* aInstance, Red::CStackFrame* aFrame, void* aOut,
                          const Context* aContext) const override;
    void ConfigureScriptFunction(Red::CClassFunction* aFunction,
                                 const ScriptablePropertySpecPtr& aPropSpec) const override;
};

/**
 * @brief A property getter that check whether a specific item is contained within an array of localization keys
 * associated with a scriptable record property.
 *
 * The generated function name for this getter has the suffix @c Contains and no prefix. For example, a property named
 * @c MyProperty would correspond to a function named @c MyPropertyContains.
 *
 * The TweakDB flat associated with the property is expected to contain an array of @c LocKey. During processing, the
 * result is validated to ensure that it is of the expected type. The getter takes a @c CName as an argument and checks
 * whether it is contained within the array of localization keys associated with the property, returning true if it is
 * and false otherwise.
 */
class LocKeyArrayContainsGetter : public TTypedPropertyGetter<GetterType::LocKeyArrayContains>
{
public:
    consteval LocKeyArrayContainsGetter()
        : TTypedPropertyGetter("Contains")
    {
    }

    [[nodiscard]] Red::CName GetFunctionHash(const ScriptableRecordSpecPtr& aRecordSpec,
                                             const ScriptablePropertySpecPtr& aPropSpec) const override;
    void HandleInvocation(Red::IScriptable* aInstance, Red::CStackFrame* aFrame, void* aOut,
                          const Context* aContext) const override;
    void ConfigureScriptFunction(Red::CClassFunction* aFunction,
                                 const ScriptablePropertySpecPtr& aPropSpec) const override;
};

/**
 * @brief A property getter that retrieves a simple value (i.e., a value that is simply returned from its TweakDB flat)
 * from a scriptable record property.
 *
 * The generated function name for this getter has no additional prefix or suffix beyond the base property name. For
 * example, a property named @c MyProperty would correspond to a function named @c MyProperty.
 *
 * The TweakDB flat associated with the property is expected to contain a value of a simple type (e.g., int, float,
 * bool, enum, arrays, etc.) that can be directly returned without additional processing. During processing, the result
 * is validated to ensure that it is of the expected type as defined by the property specification. On success, the
 * getter will return the value from TweakDB.
 */
class ValueGetter : public TTypedPropertyGetter<GetterType::Get>
{
public:
    consteval ValueGetter() = default;

    [[nodiscard]] Red::CName GetFunctionHash(const ScriptableRecordSpecPtr& aRecordSpec,
                                             const ScriptablePropertySpecPtr& aPropSpec) const override;
    void HandleInvocation(Red::IScriptable* aInstance, Red::CStackFrame* aFrame, void* aOut,
                          const Context* aContext) const override;
    void ConfigureScriptFunction(Red::CClassFunction* aFunction,
                                 const ScriptablePropertySpecPtr& aPropSpec) const override;
};

/**
 * @brief A manager for handling the registration and invocation of script functions that serve as property getters for
 * scriptable TweakDB records.
 */
class ScriptablePropertyManager
    : Core::LoggingAgent
    , Core::ShareFromThis<ScriptablePropertyManager>
{
public:
    /**
     * @brief Constructs a ScriptablePropertyManager instance with the given TweakDB manager.
     *
     * @param aManager A deferred pointer to the TweakDB manager used to retrieve scriptable record property values from
     * TweakDB at runtime.
     */
    explicit ScriptablePropertyManager(const Core::DeferredPtr<Red::TweakDBManager>& aManager);

    /**
     * @brief Registers the global invocation handler function that serves as the single entry point for all property
     * getter functions with RTTI.
     */
    void RegisterInvocationHandler();

    /**
     * @brief Checks whether a given script function corresponds to a registered property getter for a scriptable
     * record type based on a hash computed from the function's signature. If found, the function's bytecode is modified
     * to invoke the appropriate property getter at runtime.
     *
     * This process relies on a property of a scriptable record having previously been created and indexed so that the
     * hash of the function's signature can be mapped to the appropriate getter type.
     *
     * @param aRecordSpec The specification of the scriptable record type that this function belongs to.
     * @param aFunc The script function to check and potentially modify to invoke a property getter at runtime.
     * @return Whether the given function corresponds to a registered property getter and was successfully modified to
     * invoke the getter at runtime.
     */
    bool AdaptFunction(const ScriptableRecordSpecPtr& aRecordSpec, Red::CClassFunction* aFunc);

    /**
     * @brief Creates and registers script functions corresponding to the properties defined in the given scriptable
     * record specification, configuring each function to invoke the appropriate property getter at runtime based on the
     * property's specifications and the function's signature.
     *
     * @param aRecordSpec The specification of the scriptable record type for which to create and register property
     * getter functions.
     */
    void CreateFunctions(const ScriptableRecordSpecPtr& aRecordSpec);

    /**
     * @brief Creates and registers script functions corresponding to a specific property defined in the given
     * scriptable record specification, configuring each function to invoke the appropriate property getter at runtime
     * based on the property's specifications and the function's signature.
     *
     * @param aRecordSpec The specification of the scriptable record type that this property belongs to.
     * @param aPropSpec The specification of the property for which to create and register getter functions.
     */
    void CreateFunctions(const ScriptableRecordSpecPtr& aRecordSpec, const ScriptablePropertySpecPtr& aPropSpec);

    /**
     * @brief Soft-deletes script functions corresponding to the properties defined in the given scriptable record
     * specification.
     *
     * @param aRecordSpec The specification of the scriptable record type for which to delete property getter functions.
     * @param aPropSpec The specification of the property for which to delete getter functions.
     */
    void DeleteFunctions(const ScriptableRecordSpecPtr& aRecordSpec, const ScriptablePropertySpecPtr& aPropSpec);

private:
    /**
     * @brief An entry in the registry of property getter functions that contains the state of a registered property
     * getter function.
     */
    struct FunctionEntry
    {
        /**
         * @brief The type of getter function that this entry corresponds to, which determines the property getter that
         * should be invoked at runtime for functions of this type.
         */
        GetterType type;

        /**
         * @brief A pointer to the script function corresponding to this entry.
         */
        Red::CClassFunction* function;

        /**
         * @brief The buffer containing the script function's bytecode. This is stored so that the allocated memory may
         * be freed if the bytecode of the function is ever replaced, such as during a hot-reload.
         */
        Red::RawBuffer bytecode;
    };

    /**
     * @brief The name of the global function registered with RTTI that serves as the invocation handler for all
     * property getter functions.
     *
     * This function is responsible for dispatching calls to the appropriate property getter based on the execution
     * context passed to it at runtime.
     */
    static constexpr auto InvocationHandlerName = "_ScriptablePropertyInvocationHandler";

    /**
     * @brief Handles the invocation of a script function that serves as a property getter for a scriptable record type
     * by dispatching the call to the appropriate property getter based on the execution context passed to it at
     * runtime.
     *
     * This function is registered with RTTI as a global function and serves as the single entry point for all property
     * getter function calls from RedScript.
     *
     * When invoked, this function expects the @c GetterType and a pointer to the invocation's execution context, in
     * that order, as the first two elements in the call's stack frame. From the getter type enum, the handler can
     * proxy processing of the invocation to the correct specialized getter implementation. The context provides the
     * details of which record type and property are being accessed, type information, and more.
     *
     * @param aInstance The scriptable instance on which the function was invoked, expected to be an instance of a
     * scriptable record type.
     * @param aFrame The call stack frame for this function invocation, which must contain the getter type and a pointer
     * to the execution context as its first elements.
     * @param aOut A pointer to the memory location where the function's return value should be written, if it has a
     * return value.
     */
    static void HandleInvocation(Red::IScriptable* aInstance, Red::CStackFrame* aFrame, void* aOut, int64_t);

    /**
     * @brief Retrieves the script execution context from the given stack frame. After execution of this function
     * completes, the stack frame will be advanced past the location of the pointer to the execution context.
     *
     * @param aFrame The stack frame provided to the handler's invocation from which to retrieve the execution context.
     * @return A pointer to the execution context.
     */
    static Context* ParseContext(Red::CStackFrame* aFrame);

    /**
     * @brief Retrieves the type of getter function being invoked from the given stack frame. After execution of this
     * function completes, the stack frame will be advanced past the location of the getter type.
     *
     * @param aFrame The stack frame provided to the handler's invocation from which to retrieve the getter type.
     * @return The type of getter function being invoked.
     */
    static GetterType ParseGetterType(Red::CStackFrame* aFrame);

    /**
     * @brief Retrieves the property getter corresponding to the given getter type, if it has been registered.
     *
     * @param aType The type of getter function for which to retrieve the getter.
     * @return A pointer to the property getter corresponding to the given getter type
     */
    static ScriptablePropertyGetter* GetPropertyGetter(GetterType aType);

    /**
     * @brief Retrieves an array of the types of getter functions registered for the given property specification, which
     * is used to determine which property getters to create functions for when processing a property specification.
     *
     * @param aPropSpec The specification of the property for which to retrieve the types of registered getter
     * functions.
     * @return An array of the types of getter functions registered for the given property specification.
     */
    std::span<const GetterType> GetGetterTypes(const ScriptablePropertySpecPtr& aPropSpec);

    /**
     * @brief Retrieves the script execution context corresponding to the given record and property specifications,
     * creating and registering it if it does not already exist. The context is used to store necessary information for
     * the execution of a property getter, such as the property specification and pointers to necessary services.
     *
     * The context is unique to a record and property combination but is shared between all functions created for the
     * property.
     *
     * @param aRecordSpec The specification of the scriptable record type to create a context for.
     * @param aPropSpec The specification of the property to create a context for.
     * @return A shared pointer to the script execution context corresponding to the given record and property
     * specifications. If the context did not already exist, it will be created and registered before being returned.
     */
    const Context* GetContext(const ScriptableRecordSpecPtr& aRecordSpec, const ScriptablePropertySpecPtr& aPropSpec);

    /**
     * @brief Creates and registers a script function corresponding to a specific property defined in the given
     * scriptable record specification, configuring the function to invoke the appropriate property getter at runtime
     * based on the property's specifications and the function's signature.
     *
     * @param aType The type of getter function to create, used to determine which property getter to invoke at runtime.
     * @param aRecordSpec The specification of the scriptable record type that this property belongs to.
     * @param aPropSpec The specification of the property for which to create and register a getter function.
     * @return Whether the function was successfully created and registered.
     */
    bool CreateFunction(GetterType aType, const ScriptableRecordSpecPtr& aRecordSpec,
                        const ScriptablePropertySpecPtr& aPropSpec);

    /**
     * @brief Soft-deletes a script function corresponding to a specific property defined in the given scriptable record
     * specification. The function's script bytecode will be truncated and the previous bytecode buffer will be freed as
     * part of this process.
     *
     * The function is not removed from RTTI, as this would lead to the risk of dangling references elsewhere in the
     * game engine and scripting layer.
     *
     * @param aType The type of getter function to delete, used to determine which property getter function to delete.
     * @param aRecordSpec The specification of the scriptable record type that this property belongs to.
     * @param aPropSpec The specification of the property for which to delete the getter function.
     * @return Whether the function was successfully deleted.
     */
    bool DeleteFunction(GetterType aType, const ScriptableRecordSpecPtr& aRecordSpec,
                        const ScriptablePropertySpecPtr& aPropSpec);

    /**
     * @brief Modifies a given script function to invoke the appropriate property getter for a scriptable record type
     * at runtime based on the given record specification and the function's signature hash. This involves replacing the
     * function's bytecode with bytecode generated to invoke the appropriate property getter and passing the necessary
     * execution context to the getter at runtime.
     *
     * @param aFunction The script function to modify to invoke a property getter at runtime.
     * @param aEntry The function entry containing the getter type and existing bytecode for the given function, which
     * will be replaced with bytecode to invoke the appropriate property getter at runtime.
     * @param aContext The execution context to pass to the property getter at runtime.
     */
    void ReplaceByteCode(Red::CClassFunction* aFunction, const Core::SharedPtr<FunctionEntry>& aEntry,
                         const Context* aContext) const;

    /**
     * @brief Soft-deletes the bytecode of a given script function by freeing the memory allocated for the bytecode and
     * clearing the bytecode buffer in the function entry.
     *
     * This is used when a function is deleted from the registry to free the memory allocated for its bytecode, while
     * the function itself remains registered with RTTI to avoid dangling references.
     *
     * @param aEntry The function entry containing the bytecode to delete, which will be freed and cleared from the
     * entry.
     */
    void TruncateByteCode(const Core::SharedPtr<FunctionEntry>& aEntry);

    /**
     * @brief Generates the script bytecode for a given script function to invoke a property getter for a scriptable
     * record type at runtime based on the given execution context and native function object corresponding to the
     * property getter. This is used to replace the bytecode of a script function with bytecode that invokes the
     * appropriate property getter at runtime.
     *
     * The function does not modify the given script function directly, but it does inspect it to ensure the correct
     * bytecode layout for its function signature.
     *
     * @param aGetterType The type of getter function to generate bytecode for, used to determine which property getter
     * to invoke at runtime.
     * @param aContext The execution context containing necessary information for the execution of the property getter
     * function, such as the property specification and pointers to necessary services.
     * @param aFunction The script function for which to generate the bytecode to invoke a property getter at runtime.
     * @return The generated bytecode for the given script function to invoke a property getter for a scriptable record
     * type at runtime based on the given execution context and native function object corresponding to the property
     * getter.
     */
    Red::RawBuffer CreateByteCode(GetterType aGetterType, const Context* aContext,
                                  Red::CClassFunction* aFunction) const;

    /**
     * @brief Generates the hash of the function name corresponding to the given record specification and function for a
     * property getter. This hash is used to establish a link between a function parsed from RedScript to the property
     * specification associated with it based on the function's signature, which allows the correct property getter to
     * be determined for the function.
     *
     * The function name is generated based on the naming convention defined by the property getter's prefix and
     * suffix, as well as the function name specified in the property specification. For example, if the prefix is @c
     * Get and the suffix is @c Item, a property with the function name @c MyProperty would correspond to a function
     * named @c GetMyPropertyItem.
     *
     * The hash is computed from a semi-color delimited string of various string segments. The segment order is as
     * follows:
     *
     * - Record class name.
     * - Function return type, or "void" if it does not have a return.
     * - Generated function name for the getter.
     * - Types for all function arguments, sequentially, if the function has any.
     *
     * @param aRecordSpec The specification of the scriptable record type that this function belongs to.
     * @param aFunction The RedScript script function for which to generate the hash of the function name corresponding
     * to the given record specification and function for a property getter.
     * @return The hash of the function name corresponding to the given record specification and function for a property
     * getter.
     */
    Red::CName GetFunctionHash(const ScriptableRecordSpecPtr& aRecordSpec, Red::CClassFunction* aFunction);

    /**
     * @brief A deferred pointer to the TweakDB manager used to retrieve scriptable record property values from TweakDB
     * at runtime. This is used by property getters to access TweakDB when retrieving property values for scriptable
     * records.
     */
    Core::DeferredPtr<Red::TweakDBManager> m_manager;

    /**
     * @brief A pointer to the RTTI system used to register the global invocation handler function and generate bytecode
     * that invokes it at runtime. This is used to interact with the scripting system for handling the invocation of
     * property getter functions from RedScript.
     */
    Red::CRTTISystem* m_rtti;

    /**
     * @brief A pointer to the global function registered with RTTI that serves as the invocation handler for all
     * property getter functions. The registration is stored so that it can be referenced in a scripted function's
     * dynamically-generated bytecode.
     */
    Red::CGlobalFunction* m_invocationHandler = nullptr;

    /**
     * @brief A mutex for synchronizing access to the mapping of function signature hashes to getter types for
     * thread safety.
     */
    std::shared_mutex m_functionsMutex;

    /**
     * @brief A mapping of function signature hashes to function state for registered property getter functions.
     *
     * This is used to map parsed RedScript functions to the appropriate getter function and to store allocated buffers
     * to avoid memory leaks.
     */
    Core::Map<Red::CName, Core::Map<Red::CName, Core::SharedPtr<FunctionEntry>>> m_functions;

    /**
     * @brief A mutex for synchronizing access to the scriptable record property getter function execution contexts for
     * thread safety.
     */
    mutable std::shared_mutex m_contextsMutex;

    /**
     * @brief A collection of shared pointers to getter function execution contexts, kept alive for the lifetime of
     * this manager so that the raw pointers baked into bytecode remain valid.
     */
    Core::Map<Red::CName, Core::Map<Red::CName, const Context*>> m_contexts;

    static inline RecordArrayGetter s_recordArrayGetter{};
    static inline RecordArrayContainsGetter s_recordArrayContainsGetter{};
    static inline RecordItemGetter s_recordItemGetter{};
    static inline RecordItemHandleGetter s_recordItemHandleGetter{};
    static inline RecordGetter s_recordGetter{};
    static inline RecordHandleGetter s_recordHandleGetter{};
    static inline ArrayCountGetter s_arrayCountGetter{};
    static inline ArrayItemGetter s_arrayItemGetter{};
    static inline ArrayContainsGetter s_arrayContainsGetter{};
    static inline ResRefArrayGetter s_resRefArrayGetter{};
    static inline ResRefItemGetter s_resRefItemGetter{};
    static inline ResRefGetter s_resRefGetter{};
    static inline LocKeyGetter s_locKeyGetter{};
    static inline LocKeyArrayGetter s_locKeyArrayGetter{};
    static inline LocKeyItemGetter s_locKeyItemGetter{};
    static inline LocKeyArrayContainsGetter s_locKeyArrayContainsGetter{};
    static inline ValueGetter s_valueGetter{};
};

/**
 * @brief The set of getter function types associated with TweakDB record properties that are arrays of foreign keys to
 * other TweakDB records.
 */
// clang-format off
static constexpr inline auto ForeignKeyArrayGetters = std::array {
    GetterType::GetRecordArray,
    GetterType::GetArrayCount,
    GetterType::GetRecordItem,
    GetterType::GetRecordItemHandle,
    GetterType::RecordArrayContains
};
// clang-format on

/**
 * @brief The set of getter function types associated with TweakDB record properties that are a single foreign key to
 * another TweakDB record rather than an array.
 */
// clang-format off
static constexpr inline auto ForeignKeyGetters = std::array {
    GetterType::GetRecord,
    GetterType::GetRecordHandle
};
// clang-format on

/**
 * @brief The set of getter function types associated with TweakDB record properties that are arrays of resource
 * references.
 */
// clang-format off
static constexpr inline auto ResRefArrayGetters = std::array {
    GetterType::GetResRefArray,
    GetterType::GetArrayCount,
    GetterType::GetResRefItem
};
// clang-format on

/**
 * @brief The set of getter function types associated with TweakDB record properties that are a single resource
 * reference rather than an array.
 */
// clang-format off
static constexpr inline auto ResRefGetters = std::array {
    GetterType::GetResRef
};
// clang-format on

/**
 * @brief The set of getter function types associated with TweakDB record properties that are arrays of simple values
 * that are not foreign keys to other TweakDB records or resource references.
 */
// clang-format off
static constexpr inline auto ArrayGetters = std::array {
    GetterType::Get,
    GetterType::GetArrayCount,
    GetterType::GetArrayItem,
    GetterType::ArrayContains
};
// clang-format on

/**
 * @brief The set of getter function types associated with TweakDB record properties that are a single localization key.
 */
// clang-format off
static constexpr inline auto LocKeyGetters = std::array {
    GetterType::GetLocKey
};
// clang-format on

/**
 * @brief The set of getter function types associated with TweakDB record properties that are arrays of localization
 * keys.
 */
// clang-format off
static constexpr inline auto LocKeyArrayGetters = std::array {
    GetterType::GetLocKeyArray,
    GetterType::GetArrayCount,
    GetterType::GetLocKeyItem,
    GetterType::LocKeyArrayContains
};
// clang-format on

/**
 * @brief The set of getter function types associated with TweakDB record properties that are simple values that are not
 * foreign keys to other TweakDB records or resource references and can be directly returned from their TweakDB flat
 * without additional processing.
 */
// clang-format off
static constexpr inline auto ValueGetters = std::array {
    GetterType::Get
};
//clang-format on

} // namespace App
