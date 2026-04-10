#pragma once

namespace Red
{
/**
 * @brief Gets the RTTI-type of a TweakDB flat type by its type hash if the hash represents a valid TweakDB flat type.
 *
 * @param aType The hash of the flat type to get.
 * @return The RTTI type corresponding to the given flat type hash, or nullptr if the type is not a valid TweakDB flat
 * type.
 */
CBaseRTTIType* GetFlatType(uint64_t aType);

/**
 * @brief Gets the RTTI-type of a TweakDB flat type by its name if the name represents a valid TweakDB flat type.
 *
 * @param aTypeName The name of the flat type to get.
 * @return The RTTI type corresponding to the given flat type name, or nullptr if the type is not a valid TweakDB flat
 * type.
 */
CBaseRTTIType* GetFlatType(CName aTypeName);

/**
 * @brief Gets the RTTI-type of a TweakDB array type corresponding to the given element type if the name represents a
 * valid TweakDB flat element type.
 *
 * @param aTypeName The name of the element type to get the corresponding array type for.
 * @return The RTTI type of the TweakDB array type corresponding to the given element type, or nullptr if the given type
 * is not a valid TweakDB element type.
 */
CBaseRTTIType* GetArrayType(CName aTypeName);

/**
 * @brief Gets the RTTI-type of a TweakDB array type corresponding to the given element type if the name represents a
 * valid TweakDB flat element type.
 *
 * @param aType The RTTI type of the element type to get the corresponding array type for.
 * @return The RTTI type of the TweakDB array type corresponding to the given element type, or nullptr if the given type
 * is not a valid TweakDB element type.
 */
CBaseRTTIType* GetArrayType(const CBaseRTTIType* aType);

/**
 * @brief Gets the RTTI-type of a TweakDB element type corresponding to the given array type if the name represents a
 * valid TweakDB flat array type.
 *
 * @param aTypeName The name of the array type to get the corresponding element type for.
 * @return The RTTI type of the TweakDB element type corresponding to the given array type, or nullptr if the given type
 * is not a valid TweakDB array type.
 */
CBaseRTTIType* GetElementType(CName aTypeName);

/**
 * @brief Gets the RTTI-type of a TweakDB element type corresponding to the given array type if the name represents a
 * valid TweakDB flat array type.
 *
 * @param aType The RTTI type of the array type to get the corresponding element type for.
 * @return The RTTI type of the TweakDB element type corresponding to the given array type, or nullptr if the given type
 * is not a valid TweakDB array type.
 */
CBaseRTTIType* GetElementType(const CBaseRTTIType* aType);

/**
 * @brief Checks whether the given type name corresponds to a valid TweakDB flat type.
 *
 * @param aTypeName The name of the type to check.
 * @return true if the given type name corresponds to a valid TweakDB flat type, false otherwise.
 */
bool IsFlatType(CName aTypeName);

/**
 * @brief Checks whether the given RTTI type corresponds to a valid TweakDB flat type.
 *
 * @param aType The RTTI type to check.
 * @return true if the given RTTI type corresponds to a valid TweakDB flat type, false otherwise.
 */
bool IsFlatType(const CBaseRTTIType* aType);

/**
 * @brief Checks whether the given type name corresponds to a valid TweakDB array type.
 *
 * @param aTypeName The name of the type to check.
 * @return true if the given type name corresponds to a valid TweakDB array type, false otherwise.
 */
bool IsArrayType(CName aTypeName);

/**
 * @brief Checks whether the given RTTI type corresponds to a valid TweakDB array type.
 *
 * @param aType The RTTI type to check.
 * @return true if the given RTTI type corresponds to a valid TweakDB array type, false otherwise.
 */
bool IsArrayType(const CBaseRTTIType* aType);

/**
 * @brief Checks whether the given type name corresponds to a non-array TweakDB foreign key type.
 *
 * @param aTypeName The name of the type to check.
 * @return true if the given type name corresponds to a non-array TweakDB foreign key type, false otherwise.
 */
bool IsForeignKey(CName aTypeName);

/**
 * @brief Checks whether the given RTTI type corresponds to a non-array TweakDB foreign key type.
 *
 * @param aType The RTTI type to check.
 * @return true if the given RTTI type corresponds to a non-array TweakDB foreign key type, false otherwise.
 */
bool IsForeignKey(const CBaseRTTIType* aType);

/**
 * @brief Checks whether the given type name corresponds to an array TweakDB foreign key type.
 *
 * @param aTypeName The name of the type to check.
 * @return true if the given type name corresponds to an array TweakDB foreign key type, false otherwise.
 */
bool IsForeignKeyArray(CName aTypeName);

/**
 * @brief Checks whether the given RTTI type corresponds to an array TweakDB foreign key type.
 *
 * @param aType The RTTI type to check.
 * @return true if the given RTTI type corresponds to an array TweakDB foreign key type, false otherwise.
 */
bool IsForeignKeyArray(const CBaseRTTIType* aType);

/**
 * @brief Checks whether the given type name corresponds to a TweakDB resource reference token type (either "ResRef" or
 * "ScriptResRef").
 *
 * @param aTypeName The name of the type to check.
 * @return true if the given type name corresponds to a TweakDB resource reference token type, false otherwise.
 */
bool IsResRefToken(CName aTypeName);

/**
 * @brief Checks whether the given RTTI type corresponds to a TweakDB resource reference token type (either "ResRef" or
 * "ScriptResRef").
 *
 * @param aType The RTTI type to check.
 * @return true if the given RTTI type corresponds to a TweakDB resource reference token type, false otherwise.
 */
bool IsResRefToken(const CBaseRTTIType* aType);

/**
 * @brief Checks whether the given type name corresponds to a TweakDB resource reference token array type (either
 * "array:ResRef" or "array:ScriptResRef").
 *
 * @param aTypeName The name of the type to check.
 * @return true if the given type name corresponds to a TweakDB resource reference token array type, false otherwise.
 */
bool IsResRefTokenArray(CName aTypeName);

/**
 * @brief Checks whether the given RTTI type corresponds to a TweakDB resource reference token array type (either
 * "array:ResRef" or "array:ScriptResRef").
 *
 * @param aType The RTTI type to check.
 * @return true if the given RTTI type corresponds to a TweakDB resource reference token array type, false otherwise.
 */
bool IsResRefTokenArray(const CBaseRTTIType* aType);

/**
 * @brief Checks whether the given type name corresponds to a TweakDB localization key type.
 *
 * @param aTypeName The name of the type to check.
 * @return true if the given type name corresponds to a TweakDB localization key type, false otherwise.
 */
bool IsLocKey(CName aTypeName);

/**
 * @brief Checks whether the given RTTI type corresponds to a TweakDB localization key type.
 *
 * @param aType The RTTI type to check.
 * @return true if the given RTTI type corresponds to a TweakDB localization key type, false otherwise.
 */
bool IsLocKey(const CBaseRTTIType* aType);

/**
 * @brief Checks whether the given type name corresponds to a TweakDB localization key array type.
 *
 * @param aTypeName The name of the type to check.
 * @return true if the given type name corresponds to a TweakDB localization key array type, false otherwise.
 */
bool IsLocKeyArray(CName aTypeName);

/**
 * @brief Checks whether the given RTTI type corresponds to a TweakDB localization key array type.
 *
 * @param aType The RTTI type to check.
 * @return true if the given RTTI type corresponds to a TweakDB localization key array type, false otherwise.
 */
bool IsLocKeyArray(const CBaseRTTIType* aType);

/**
 * @brief Gets the name of the TweakDB array type corresponding to the given element type name.
 *
 * @param aTypeName The name of the element type to get the corresponding array type name for.
 * @return The name of the TweakDB array type corresponding to the given element type name, or an empty CName if the
 * given type name is not a valid TweakDB element type.
 */
CName GetArrayTypeName(CName aTypeName);

/**
 * @brief Gets the name of the TweakDB array type corresponding to the given element RTTI type.
 *
 * @param aType The RTTI type of the element type to get the corresponding array type name for.
 * @return The name of the TweakDB array type corresponding to the given element RTTI type, or an empty CName if the
 * given type is not a valid TweakDB element type.
 */
CName GetArrayTypeName(const CBaseRTTIType* aType);

/**
 * @brief Gets the name of the TweakDB element type corresponding to the given array type name.
 *
 * @param aTypeName The name of the array type to get the corresponding element type name for.
 * @return The name of the TweakDB element type corresponding to the given array type name, or an empty CName if the
 * given type name is not a valid TweakDB array type.
 */
CName GetElementTypeName(CName aTypeName);

/**
 * @brief Gets the name of the TweakDB element type corresponding to the given array RTTI type.
 *
 * @param aType The RTTI type of the array type to get the corresponding element type name for.
 * @return The name of the TweakDB element type corresponding to the given array RTTI type, or an empty CName if the
 * given type is not a valid TweakDB array type.
 */
CName GetElementTypeName(const CBaseRTTIType* aType);

/**
 * @brief Constructs an instance of the given TweakDB flat type by its name with its default (empty) value.
 *
 * @param aTypeName The name of the TweakDB flat type to construct an instance of.
 * @return An instance of the given TweakDB flat type, or nullptr if the given type name is not a valid TweakDB flat
 * type.
 */
InstancePtr<> ConstructFlatInstance(CName aTypeName);

/**
 * @brief Constructs an instance of the given TweakDB flat type with its default (empty) value.
 *
 * @param aType The RTTI type of the TweakDB flat type to construct an instance of.
 * @return An instance of the given TweakDB flat type, or nullptr if the given RTTI type is not a valid TweakDB flat
 * type.
 */
InstancePtr<> ConstructFlatInstance(const CBaseRTTIType* aType);

/**
 * @brief Constructs a value of the given TweakDB flat type by its name with its default (empty) value.
 *
 * @param aType The RTTI type of the TweakDB flat type to construct a value of.
 * @return A value of the given TweakDB flat type, or nullptr if the given RTTI type is not a valid TweakDB flat type.
 */
ValuePtr<> ConstructFlatValue(const CBaseRTTIType* aType);

/**
 * @brief Constructs a value of the given TweakDB flat type by its name with its default (empty) value.
 *
 * @param aTypeName The name of the TweakDB flat type to construct a value of.
 * @return A value of the given TweakDB flat type, or nullptr if the given type name is not a valid TweakDB flat type.
 */
ValuePtr<> ConstructFlatValue(CName aTypeName);

/**
 * @brief Gets the RTTI class type corresponding to the given TweakDB record type name.
 *
 * @param aTypeName The name of the TweakDB record type to get the RTTI class type for.
 * @return The RTTI class type corresponding to the given TweakDB record type name, or nullptr if the given type name
 * does not correspond to a valid TweakDB record type.
 */
CClass* GetRecordType(CName aTypeName);

/**
 * @brief Gets the RTTI class type corresponding to the given TweakDB record type name.
 *
 * @param aTypeName The name of the TweakDB record type to get the RTTI class type for.
 * @return The RTTI class type corresponding to the given TweakDB record type name, or nullptr if the given type name
 * does not correspond to a valid TweakDB record type.
 */
CClass* GetRecordType(const char* aTypeName);

/**
 * @brief Checks whether the given type name corresponds to a valid TweakDB record type, which is determined by whether
 * the type is a descendent of Red::TweakDBRecord.
 *
 * @param aTypeName The name of the type to check.
 * @return true if the given type name corresponds to a valid TweakDB record type, false otherwise.
 */
bool IsRecordType(CName aTypeName);

/**
 * @brief Checks whether the given RTTI type corresponds to a valid TweakDB record type, which is determined by whether
 * the type is a descendent of Red::TweakDBRecord.
 *
 * @param aType The RTTI type to check.
 * @return true if the given RTTI type corresponds to a valid TweakDB record type, false otherwise.
 */
bool IsRecordType(const CClass* aType);

/**
 * @brief Normalizes a TweakDB record name to conform to typical TweakDB naming standards. The name is processed as
 * follows:
 *
 * - The short name is extracted from the provided name (e.g. "foo_Record" -> "foo").
 * - The short name is capitalized (e.g. "foo" -> "Foo").
 * - The capitalized short name is converted to a fully-qualified record name by prepending "gamedata" and appending
 * "_Record" to it.
 *
 * @param aName The name of the record to normalize.
 * @return The normalized record name corresponding to the given name, or an empty string if the provided name is empty
 * or invalid.
 */
std::string NormalizeRecordName(const std::string& aName);

/**
 * @brief Gets the fully-qualified TweakDB record name corresponding to the given name. Fully-qualified TweakDB names
 * follow the convention "gamedata<Name>_Record", where "<Name>" is the capitalized short name of the record type.
 *
 * This function assumes that the name is already normalized (i.e. the short name is capitalized).
 *
 * @tparam T The type to return the record name as, either std::string or CName.
 * @param aName The name of the record to get the fully-qualified name for.
 * @return The fully-qualified record name corresponding to the given name.
 */
template<typename T>
T GetRecordFullName(const std::string& aName);

/**
 * @brief Gets the fully-qualified TweakDB record name corresponding to the given name. Fully-qualified TweakDB names
 * follow the convention "gamedata<Name>_Record", where "<Name>" is the capitalized short name of the record type.
 *
 * This function assumes that the name is already normalized (i.e. the short name is capitalized).
 *
 * @tparam T The type to return the record name as, either std::string or CName.
 * @param aName The name of the record to get the fully-qualified name for.
 * @return The fully-qualified record name corresponding to the given name.
 */
template<typename T>
T GetRecordFullName(const char* aName);

/**
 * @brief Gets the fully-qualified TweakDB record name corresponding to the given name. Fully-qualified TweakDB names
 * follow the convention "gamedata<Name>_Record", where "<Name>" is the capitalized short name of the record type.
 *
 * This function assumes that the name is already normalized (i.e. the short name is capitalized).
 *
 * @tparam T The type to return the record name as, either std::string or CName.
 * @param aName The name of the record to get the fully-qualified name for.
 * @return The fully-qualified record name corresponding to the given name.
 */
template<typename T>
T GetRecordFullName(CName aName);

/**
 * @brief Gets the alias/scripted TweakDB record name corresponding to the given name. The alias name is the same as the
 * fully-qualified name except the "gamedata" prefix is stripped (e.g. "gamedataVehicle_Record" -> "Vehicle_Record").
 *
 * This function assumes that the name is already normalized (i.e. the short name is capitalized).
 *
 * @tparam T The type to return the record name as, either std::string or CName.
 * @param aName The name of the record to get the alias/scripted name for.
 * @return The alias/scripted record name corresponding to the given name.
 */
template<typename T>
T GetRecordAliasName(const std::string& aName);

/**
 * @brief Gets the alias/scripted TweakDB record name corresponding to the given name. The alias name is the same as the
 * fully-qualified name except the "gamedata" prefix is stripped (e.g. "gamedataVehicle_Record" -> "Vehicle_Record").
 *
 * This function assumes that the name is already normalized (i.e. the short name is capitalized).
 *
 * @tparam T The type to return the record name as, either std::string or CName.
 * @param aName The name of the record to get the alias/scripted name for.
 * @return The alias/scripted record name corresponding to the given name.
 */
template<typename T>
T GetRecordAliasName(const char* aName);

/**
 * @brief Gets the alias/scripted TweakDB record name corresponding to the given name. The alias name is the same as the
 * fully-qualified name except the "gamedata" prefix is stripped (e.g. "gamedataVehicle_Record" -> "Vehicle_Record").
 *
 * This function assumes that the name is already normalized (i.e. the short name is capitalized).
 *
 * @tparam T The type to return the record name as, either std::string or CName.
 * @param aName The name of the record to get the alias/scripted name for.
 * @return The alias/scripted record name corresponding to the given name.
 */
template<typename T>
T GetRecordAliasName(CName aName);

/**
 * @brief Gets the short name of a TweakDB record corresponding to the given name. The short name is the part of the
 * record name that identifies the record type without any prefixes or suffixes (e.g. "Vehicle" for
 * "gamedataVehicle_Record").
 *
 * This function does not normalize (i.e. capitalize) the name.
 *
 * @tparam T The type to return the record name as, either std::string or CName.
 * @param aName The name of the record to get the short name for.
 * @return The short name of the record corresponding to the given name.
 */
template<typename T>
T GetRecordShortName(const std::string& aName);

/**
 * @brief Gets the short name of a TweakDB record corresponding to the given name. The short name is the part of the
 * record name that identifies the record type without any prefixes or suffixes (e.g. "Vehicle" for
 * "gamedataVehicle_Record").
 *
 * This function does not normalize (i.e. capitalize) the name.
 *
 * @tparam T The type to return the record name as, either std::string or CName.
 * @param aName The name of the record to get the short name for.
 * @return The short name of the record corresponding to the given name.
 */
template<typename T>
T GetRecordShortName(CName aName);

/**
 * @brief Gets the short name of a TweakDB record corresponding to the given name. The short name is the part of the
 * record name that identifies the record type without any prefixes or suffixes (e.g. "Vehicle" for
 * "gamedataVehicle_Record").
 *
 * This function does not normalize (i.e. capitalize) the name.
 *
 * @tparam T The type to return the record name as, either std::string or CName.
 * @param aName The name of the record to get the short name for.
 * @return The short name of the record corresponding to the given name.
 */
template<typename T>
T GetRecordShortName(const char* aName);

/**
 * @brief Gets the fully-qualified TweakDB record name corresponding to the given name. Fully-qualified TweakDB names
 * follow the convention "gamedata<Name>_Record", where "<Name>" is the capitalized short name of the record type.
 *
 * This function assumes that the name is already normalized (i.e. the short name is capitalized).
 *
 * @param aName The name of the record to get the fully-qualified name for.
 * @return The fully-qualified record name corresponding to the given name.
 */
template<>
std::string GetRecordFullName(const std::string& aName);

/**
 * @brief Gets the fully-qualified TweakDB record name corresponding to the given name. Fully-qualified TweakDB names
 * follow the convention "gamedata<Name>_Record", where "<Name>" is the capitalized short name of the record type.
 *
 * This function assumes that the name is already normalized (i.e. the short name is capitalized).
 *
 * @param aName The name of the record to get the fully-qualified name for.
 * @return The fully-qualified record name corresponding to the given name.
 */
template<>
std::string GetRecordFullName(const char* aName);

/**
 * @brief Gets the fully-qualified TweakDB record name corresponding to the given name. Fully-qualified TweakDB names
 * follow the convention "gamedata<Name>_Record", where "<Name>" is the capitalized short name of the record type.
 *
 * This function assumes that the name is already normalized (i.e. the short name is capitalized).
 *
 * @param aName The name of the record to get the fully-qualified name for.
 * @return The fully-qualified record name corresponding to the given name.
 */
template<>
std::string GetRecordFullName(CName aName);

/**
 * @brief Gets the fully-qualified TweakDB record name corresponding to the given name. Fully-qualified TweakDB names
 * follow the convention "gamedata<Name>_Record", where "<Name>" is the capitalized short name of the record type.
 *
 * This function assumes that the name is already normalized (i.e. the short name is capitalized).
 *
 * @param aName The name of the record to get the fully-qualified name for.
 * @return The fully-qualified record name corresponding to the given name.
 */
template<>
CName GetRecordFullName(const std::string& aName);

/**
 * @brief Gets the fully-qualified TweakDB record name corresponding to the given name. Fully-qualified TweakDB names
 * follow the convention "gamedata<Name>_Record", where "<Name>" is the capitalized short name of the record type.
 *
 * This function assumes that the name is already normalized (i.e. the short name is capitalized).
 *
 * @param aName The name of the record to get the fully-qualified name for.
 * @return The fully-qualified record name corresponding to the given name.
 */
template<>
CName GetRecordFullName(const char* aName);

/**
 * @brief Gets the fully-qualified TweakDB record name corresponding to the given name. Fully-qualified TweakDB names
 * follow the convention "gamedata<Name>_Record", where "<Name>" is the capitalized short name of the record type.
 *
 * This function assumes that the name is already normalized (i.e. the short name is capitalized).
 *
 * @param aName The name of the record to get the fully-qualified name for.
 * @return The fully-qualified record name corresponding to the given name.
 */
template<>
CName GetRecordFullName(CName aName);

/**
 * @brief Gets the alias/scripted TweakDB record name corresponding to the given name. The alias name is the same as the
 * fully-qualified name except the "gamedata" prefix is stripped (e.g. "gamedataVehicle_Record" -> "Vehicle_Record").
 *
 * This function assumes that the name is already normalized (i.e. the short name is capitalized).
 *
 * @param aName The name of the record to get the alias/scripted name for.
 * @return The alias/scripted record name corresponding to the given name.
 */
template<>
std::string GetRecordAliasName(const std::string& aName);

/**
 * @brief Gets the alias/scripted TweakDB record name corresponding to the given name. The alias name is the same as the
 * fully-qualified name except the "gamedata" prefix is stripped (e.g. "gamedataVehicle_Record" -> "Vehicle_Record").
 *
 * This function assumes that the name is already normalized (i.e. the short name is capitalized).
 *
 * @param aName The name of the record to get the alias/scripted name for.
 * @return The alias/scripted record name corresponding to the given name.
 */
template<>
std::string GetRecordAliasName(const char* aName);

/**
 * @brief Gets the alias/scripted TweakDB record name corresponding to the given name. The alias name is the same as the
 * fully-qualified name except the "gamedata" prefix is stripped (e.g. "gamedataVehicle_Record" -> "Vehicle_Record").
 *
 * This function assumes that the name is already normalized (i.e. the short name is capitalized).
 *
 * @param aName The name of the record to get the alias/scripted name for.
 * @return The alias/scripted record name corresponding to the given name.
 */
template<>
std::string GetRecordAliasName(CName aName);

/**
 * @brief Gets the alias/scripted TweakDB record name corresponding to the given name. The alias name is the same as the
 * fully-qualified name except the "gamedata" prefix is stripped (e.g. "gamedataVehicle_Record" -> "Vehicle_Record").
 *
 * This function assumes that the name is already normalized (i.e. the short name is capitalized).
 *
 * @param aName The name of the record to get the alias/scripted name for.
 * @return The alias/scripted record name corresponding to the given name.
 */
template<>
CName GetRecordAliasName(const std::string& aName);

/**
 * @brief Gets the alias/scripted TweakDB record name corresponding to the given name. The alias name is the same as the
 * fully-qualified name except the "gamedata" prefix is stripped (e.g. "gamedataVehicle_Record" -> "Vehicle_Record").
 *
 * This function assumes that the name is already normalized (i.e. the short name is capitalized).
 *
 * @param aName The name of the record to get the alias/scripted name for.
 * @return The alias/scripted record name corresponding to the given name.
 */
template<>
CName GetRecordAliasName(const char* aName);

/**
 * @brief Gets the alias/scripted TweakDB record name corresponding to the given name. The alias name is the same as the
 * fully-qualified name except the "gamedata" prefix is stripped (e.g. "gamedataVehicle_Record" -> "Vehicle_Record").
 *
 * This function assumes that the name is already normalized (i.e. the short name is capitalized).
 *
 * @param aName The name of the record to get the alias/scripted name for.
 * @return The alias/scripted record name corresponding to the given name.
 */
template<>
CName GetRecordAliasName(CName aName);

/**
 * @brief Gets the short name of a TweakDB record corresponding to the given name. The short name is the part of the
 * record name that identifies the record type without any prefixes or suffixes (e.g. "Vehicle" for
 * "gamedataVehicle_Record").
 *
 * This function does not normalize (i.e. capitalize) the name.
 *
 * @param aName The name of the record to get the short name for.
 * @return The short name of the record corresponding to the given name.
 */
template<>
std::string GetRecordShortName(const std::string& aName);

/**
 * @brief Gets the short name of a TweakDB record corresponding to the given name. The short name is the part of the
 * record name that identifies the record type without any prefixes or suffixes (e.g. "Vehicle" for
 * "gamedataVehicle_Record").
 *
 * This function does not normalize (i.e. capitalize) the name.
 *
 * @param aName The name of the record to get the short name for.
 * @return The short name of the record corresponding to the given name.
 */
template<>
std::string GetRecordShortName(CName aName);

/**
 * @brief Gets the short name of a TweakDB record corresponding to the given name. The short name is the part of the
 * record name that identifies the record type without any prefixes or suffixes (e.g. "Vehicle" for
 * "gamedataVehicle_Record").
 *
 * This function does not normalize (i.e. capitalize) the name.
 *
 * @param aName The name of the record to get the short name for.
 * @return The short name of the record corresponding to the given name.
 */
template<>
std::string GetRecordShortName(const char* aName);

/**
 * @brief Gets the short name of a TweakDB record corresponding to the given name. The short name is the part of the
 * record name that identifies the record type without any prefixes or suffixes (e.g. "Vehicle" for
 * "gamedataVehicle_Record").
 *
 * This function does not normalize (i.e. capitalize) the name.
 *
 * @param aName The name of the record to get the short name for.
 * @return The short name of the record corresponding to the given name.
 */
template<>
CName GetRecordShortName(const std::string& aName);

/**
 * @brief Gets the short name of a TweakDB record corresponding to the given name. The short name is the part of the
 * record name that identifies the record type without any prefixes or suffixes (e.g. "Vehicle" for
 * "gamedataVehicle_Record").
 *
 * This function does not normalize (i.e. capitalize) the name.
 *
 * @param aName The name of the record to get the short name for.
 * @return The short name of the record corresponding to the given name.
 */
template<>
CName GetRecordShortName(CName aName);

/**
 * @brief Gets the short name of a TweakDB record corresponding to the given name. The short name is the part of the
 * record name that identifies the record type without any prefixes or suffixes (e.g. "Vehicle" for
 * "gamedataVehicle_Record").
 *
 * This function does not normalize (i.e. capitalize) the name.
 *
 * @param aName The name of the record to get the short name for.
 * @return The short name of the record corresponding to the given name.
 */
template<>
CName GetRecordShortName(const char* aName);

/**
 * @brief Gets the hash of a TweakDB record type corresponding to the given name. The name is assumed to be a
 * fully-qualified TweakDB record type name and is converted to its short name for hashing.
 *
 * It is the responsibility of the caller to ensure that the given string is a valid TweakDB record type name and to do
 * any necessary normalization (e.g. capitalization) before calling this function.
 *
 * @param aName The fully-qualified name of the record type to get the hash for.
 * @return The hash of the TweakDB record type corresponding to the given name, or 0 if the given name is empty or
 * invalid.
 */
uint32_t GetRecordTypeHash(CName aName);

/**
 * @brief Gets the hash of a TweakDB record type corresponding to the given name. The name is assumed to be a
 * valid TweakDB record type short name and is not modified before hashing.
 *
 * It is the responsibility of the caller to ensure that the given string is a valid TweakDB record type name and to do
 * any necessary normalization (e.g. capitalization) before calling this function.
 *
 * @param aName The short name of the record type to get the hash for.
 * @return The hash of the TweakDB record type corresponding to the given name, or 0 if the given name is empty or
 * invalid.
 */
uint32_t GetRecordTypeHash(const std::string& aName);

/**
 * @brief Gets the hash of a TweakDB record type corresponding to the given name. The name is assumed to be a
 * valid TweakDB record type short name and is not modified before hashing.
 *
 * It is the responsibility of the caller to ensure that the given string is a valid TweakDB record type name and to do
 * any necessary normalization (e.g. capitalization) before calling this function.
 *
 * @param aName The short name of the record type to get the hash for.
 * @return The hash of the TweakDB record type corresponding to the given name, or 0 if the given name is empty or
 * invalid.
 */
uint32_t GetRecordTypeHash(const char* aName);

/**
 * @brief Gets the hash of a TweakDB record type corresponding to the given RTTI class type. The class is assumed to be
 * a valid TweakDB record type and its name is converted to the short name for hashing.
 *
 * @param aType The RTTI class type of the record type to get the hash for.
 * @return The hash of the TweakDB record type corresponding to the given RTTI class type, or 0 if the given type is not
 * a valid TweakDB record type.
 */
uint32_t GetRecordTypeHash(const CClass* aType);

/**
 * @brief Gets the TweakDBID of the flat containing the default value for a TweakDB property corresponding to the given
 * record and property names. The record name is assumed to be a valid TweakDB record type name and is converted to the
 * record's short name. The property name is assumed to be a valid property of the given record type and is not modified
 * before lookup.
 *
 * The resulting TweakDBID can be expected to contain the follow path components joined by period characters:
 *
 * - "RTDB"
 * - Short name of the record.
 * - Property name.
 *
 * For example, the property "displayName" of record type "gamedataVehicle_Record" will result in a TweakDBID for the
 * path "RTDB.Vehicle.displayName".
 *
 * @param aRecord The name of the record type to get the property TweakDBID for.
 * @param aProp The name of the property to get the TweakDBID for.
 * @return The TweakDBID of the default value for the given record and property, or an empty TweakDBID if the given
 * record or property name is empty or invalid.
 */
TweakDBID GetRTDBFlatID(CName aRecord, const std::string& aProp);

/**
 * @brief Gets the TweakDBID of the flat containing the default value for a TweakDB property corresponding to the given
 * record and property names. The record name is assumed to be a valid TweakDB record type name and is converted to the
 * record's short name. The property name is assumed to be a valid property of the given record type and is not modified
 * before lookup.
 *
 * The resulting TweakDBID can be expected to contain the follow path components joined by period characters:
 *
 * - "RTDB"
 * - Short name of the record.
 * - Property name.
 *
 * For example, the property "displayName" of record type "gamedataVehicle_Record" will result in a TweakDBID for the
 * path "RTDB.Vehicle.displayName".
 *
 * @param aRecord The name of the record type to get the property TweakDBID for.
 * @param aProp The name of the property to get the TweakDBID for.
 * @return The TweakDBID of the default value for the given record and property, or an empty TweakDBID if the given
 * record or property name is empty or invalid.
 */
TweakDBID GetRTDBFlatID(CName aRecord, const char* aProp);

/**
 * @brief Gets the TweakDBID of the record containing the default values for a TweakDB record corresponding to the given
 * record name. The record name is assumed to be a valid TweakDB record type name and is converted to the record's
 * short name.
 *
 * The resulting TweakDBID may be used as the prefix for the flats containing default values for properties of the
 * record. It can be expected to contain the follow path components joined by period characters:
 *
 * - "RTDB"
 * - Short name of the record.
 *
 * For example, for record type "gamedataVehicle_Record", the resulting TweakDBID will contain the path "RTDB.Vehicle"
 * and the TweakDBIDs for the default value flats for properties of this record will be prefixed by this record
 * TweakDBID (e.g. "RTDB.Vehicle.displayName").
 *
 * @param aRecord The name of the record type to get the record TweakDBID for.
 * @return The TweakDBID of the record containing the default values for the given record, or an empty TweakDBID if the
 * given record name is empty or invalid.
 */
TweakDBID GetRTDBRecordID(const std::string& aRecord);

/**
 * @brief Gets the TweakDBID of the record containing the default values for a TweakDB record corresponding to the given
 * record name. The record name is assumed to be a valid TweakDB record type name and is converted to the record's
 * short name.
 *
 * The resulting TweakDBID may be used as the prefix for the flats containing default values for properties of the
 * record. It can be expected to contain the follow path components joined by period characters:
 *
 * - "RTDB"
 * - Short name of the record.
 *
 * For example, for record type "gamedataVehicle_Record", the resulting TweakDBID will contain the path "RTDB.Vehicle"
 * and the TweakDBIDs for the default value flats for properties of this record will be prefixed by this record
 * TweakDBID (e.g. "RTDB.Vehicle.displayName").
 *
 * @param aRecord The name of the record type to get the record TweakDBID for.
 * @return The TweakDBID of the record containing the default values for the given record, or an empty TweakDBID if the
 * given record name is empty or invalid.
 */
TweakDBID GetRTDBRecordID(CName aRecord);

/**
 * @brief Capitalizes the given name by converting the first character to uppercase if it is a lowercase letter. For
 * example, "foo" will be capitalized to "Foo", while "Foo" and "123abc" will remain unchanged.
 *
 * @param aName The name to capitalize.
 * @return The capitalized version of the given name.
 */
std::string Capitalize(const std::string& aName);

/**
 * @brief Capitalizes the given name by converting the first character to uppercase if it is a lowercase letter. For
 * @brief Capitalizes the given name by converting the first character to uppercase if it is a lowercase letter. For
 * example, "foo" will be capitalized to "Foo", while "Foo" and "123abc" will remain unchanged.
 *
 * @param aName The name to capitalize.
 * @return The capitalized version of the given name.
 */
std::string Capitalize(const char* aName);

/**
 * @brief Decapitalizes the given name by converting the first character to lowercase if it is an uppercase letter. For
 * example, "Foo" will be decapitalized to "foo", while "foo" and "123abc" will remain unchanged.
 *
 * @param aName The name to decapitalize.
 * @return The decapitalized version of the given name.
 */
std::string Decapitalize(CName aName);

/**
 * @brief Decapitalizes the given name by converting the first character to lowercase if it is an uppercase letter. For
 * example, "Foo" will be decapitalized to "foo", while "foo" and "123abc" will remain unchanged.
 *
 * @param aName The name to decapitalize.
 * @return The decapitalized version of the given name.
 */
std::string Decapitalize(const std::string& aName);

/**
 * @brief Decapitalizes the given name by converting the first character to lowercase if it is an uppercase letter. For
 * example, "Foo" will be decapitalized to "foo", while "foo" and "123abc" will remain unchanged.
 *
 * @param aName The name to decapitalize.
 * @return The decapitalized version of the given name.
 */
std::string Decapitalize(const char* aName);

/**
 * @brief Gets the name of the handle type corresponding to the given class type.
 *
 * @tparam T The type to return the handle type name as, either std::string or CName.
 * @param aClass The class type to get the handle type name for.
 * @return The name of the handle type corresponding to the given class type.
 */
template<typename T>
T GetHandleTypeName(const CClass* aClass);

/**
 * @brief Gets the name of the weak handle type corresponding to the given class type.
 *
 * @tparam T The type to return the weak handle type name as, either std::string or CName.
 * @param aClass The class type to get the weak handle type name for.
 * @return The name of the weak handle type corresponding to the given class type.
 */
template<typename T>
T GetWHandleTypeName(const CClass* aClass);

/**
 * @brief Gets the name of the weak handle array type corresponding to the given class type.
 *
 * @tparam T The type to return the weak handle array type name as, either std::string or CName.
 * @param aClass The class type to get the weak handle array type name for.
 * @return The name of the weak handle array type corresponding to the given class type.
 */
template<typename T>
T GetWHandleArrayTypeName(const CClass* aClass);

/**
 * @brief Gets the name of the handle type corresponding to the given class type.
 *
 * @param aClass The class type to get the handle type name for.
 * @return The name of the handle type corresponding to the given class type.
 */
template<>
std::string GetHandleTypeName(const CClass* aClass);

/**
 * @brief Gets the name of the weak handle type corresponding to the given class type.
 *
 * @param aClass The class type to get the weak handle type name for.
 * @return The name of the weak handle type corresponding to the given class type.
 */
template<>
std::string GetWHandleTypeName(const CClass* aClass);

/**
 * @brief Gets the name of the weak handle array type corresponding to the given class type.
 *
 * @param aClass The class type to get the weak handle array type name for.
 * @return The name of the weak handle array type corresponding to the given class type.
 */
template<>
std::string GetWHandleArrayTypeName(const CClass* aClass);

/**
 * @brief Gets the name of the handle type corresponding to the given class type.
 *
 * @param aClass The class type to get the handle type name for.
 * @return The name of the handle type corresponding to the given class type.
 */
template<>
CName GetHandleTypeName(const CClass* aClass);

/**
 * @brief Gets the name of the weak handle type corresponding to the given class type.
 *
 * @param aClass The class type to get the weak handle type name for.
 * @return The name of the weak handle type corresponding to the given class type.
 */
template<>
CName GetWHandleTypeName(const CClass* aClass);

/**
 * @brief Gets the name of the weak handle array type corresponding to the given class type.
 *
 * @param aClass The class type to get the weak handle array type name for.
 * @return The name of the weak handle array type corresponding to the given class type.
 */
template<>
CName GetWHandleArrayTypeName(const CClass* aClass);

/**
 * @brief Gets the handle type corresponding to the given class type.
 *
 * @param aClass The class type to get the handle type for.
 * @return The handle type corresponding to the given class type, or nullptr if the handle type does not exist.
 */
CHandle* GetHandleType(const CClass* aClass);

/**
 * @brief Gets the weak handle type corresponding to the given class type.
 *
 * @param aClass The class type to get the weak handle type for.
 * @return The weak handle type corresponding to the given class type, or nullptr if the weak handle type does not
 * exist.
 */
CWeakHandle* GetWHandleType(const CClass* aClass);

/**
 * @brief Gets the weak handle array type corresponding to the given class type.
 *
 * @param aClass The class type to get the weak handle array type for.
 * @return The weak handle array type corresponding to the given class type, or nullptr if the weak handle array type
 * does not exist.
 */
CRTTIBaseArrayType* GetWHandleArrayType(const CClass* aClass);

/**
 * @brief Attempts to cast and return the given type as an array type. If the type is not an array, @c nullptr is
 * returned.
 *
 * @param aType The type to cast to an array type.
 * @return The given type cast as an array type, or @c nullptr if the type is not an array.
 */
const CRTTIBaseArrayType* ToArrayType(const CBaseRTTIType* aType);

/**
 * @brief Attempts to cast and return the given type as a weak handle type. If the type is not a weak handle, @c nullptr
 * is returned.
 *
 * @param aType The type to cast to a weak handle type.
 * @return The given type cast as a weak handle type, or @c nullptr if the type is not a weak handle.
 */
const CWeakHandle* ToWeakHandleType(const CBaseRTTIType* aType);

/**
 * @brief Attempts to cast and return the given type as a handle type. If the type is not a handle, @c nullptr is
 * returned.
 *
 * @param aType The type to cast to a handle type.
 * @return The given type cast as a handle type, or @c nullptr if the type is not a handle.
 */
const CHandle* ToHandleType(const CBaseRTTIType* aType);

/**
 * @brief Gets the inner type of the given type if it is an array, handle, or weak handle type.
 *
 * For array types, the inner type is the element type. For handle and weak handle types, the inner type is the class
 * type that the handle refers to.
 *
 * If the given type is not an array, handle, or weak handle type, @c nullptr is returned.
 *
 * @param aType The type to get the inner type of.
 * @return The inner type of the given type, or @c nullptr .
 */
template<typename T = CBaseRTTIType>
    requires std::is_base_of_v<CBaseRTTIType, T>
const T* GetInnerType(const CBaseRTTIType* aType);

/**
 * @brief Gets the inner type of the given type if it is an array, handle, or weak handle type.
 *
 * @param aType The type to get the inner type of.
 * @return The inner type of the given type, or @c nullptr.
 */
template<>
const CBaseRTTIType* GetInnerType(const CBaseRTTIType* aType);

/**
 * @brief Gets the innermost class type of the given type if it is an array, handle, or weak handle type.
 *
 * If the type is an array of handles, the returned type will be in the inner type of the handle elements of the array.
 *
 * @param aType The type to get the inner type of.
 * @return The inner class type of the given type, or @c nullptr.
 */
template<>
const CClass* GetInnerType(const CBaseRTTIType* aType);

/**
 * @brief Gets the name of the inner type of the given type if it is an array, handle, or weak handle type.
 *
 * For array types, the inner type is the element type. For handle and weak handle types, the inner type is the class
 * type that the handle refers to.
 *
 * If the given type is not an array, handle, or weak handle type, an empty @c CName is returned.
 *
 * @param aType The type to get the inner type name of.
 * @return The name of the inner type of the given type, or empty.
 */
CName GetInnerTypeName(const CBaseRTTIType* aType);
} // namespace Red
