#include "Types.hpp"

#include <RED4ext/NativeTypes.hpp>
#include <RED4ext/ResourceReference.hpp>
#include <RED4ext/RTTISystem.hpp>
#include <RED4ext/Scripting/Natives/Generated/Color.hpp>
#include <RED4ext/Scripting/Natives/Generated/EulerAngles.hpp>
#include <RED4ext/Scripting/Natives/Generated/Vector2.hpp>
#include <RED4ext/Scripting/Natives/Generated/Vector3.hpp>
#include <RED4ext/Scripting/Natives/Generated/Quaternion.hpp>

namespace
{
constexpr Red::CName BaseRecordTypeName = "gamedataTweakDBRecord";
constexpr Red::CName ResRefTokenTypeName = "redResourceReferenceScriptToken";
constexpr Red::CName ResRefTokenArrayTypeName = "array:redResourceReferenceScriptToken";
constexpr const char* RecordTypePrefix = "gamedata";
constexpr const char* RecordTypeSuffix = "_Record";
constexpr size_t RecordTypePrefixLength = std::char_traits<char>::length(RecordTypePrefix);
constexpr size_t RecordTypeSuffixLength = std::char_traits<char>::length(RecordTypeSuffix);
constexpr const char* DefaultsGroup = "RTDB.";
constexpr const char* PropSeparator = ".";
}

bool Red::TweakDB::IsFlatType(Red::CName aTypeName)
{
    switch (aTypeName)
    {
    case EFlatType::Int32:
    case EFlatType::Float:
    case EFlatType::Bool:
    case EFlatType::String:
    case EFlatType::CName:
    case EFlatType::TweakDBID:
    case EFlatType::LocKey:
    case EFlatType::Resource:
    case EFlatType::Quaternion:
    case EFlatType::EulerAngles:
    case EFlatType::Vector3:
    case EFlatType::Vector2:
    case EFlatType::Color:
    case EFlatType::Int32Array:
    case EFlatType::FloatArray:
    case EFlatType::BoolArray:
    case EFlatType::StringArray:
    case EFlatType::CNameArray:
    case EFlatType::TweakDBIDArray:
    case EFlatType::LocKeyArray:
    case EFlatType::ResourceArray:
    case EFlatType::QuaternionArray:
    case EFlatType::EulerAnglesArray:
    case EFlatType::Vector3Array:
    case EFlatType::Vector2Array:
    case EFlatType::ColorArray:
        return true;
    default:
        return false;
    }
}

bool Red::TweakDB::IsFlatType(const Red::CBaseRTTIType* aType)
{
    return aType && IsFlatType(aType->GetName());
}

bool Red::TweakDB::IsRecordType(Red::CName aTypeName)
{
    return IsRecordType(Red::CRTTISystem::Get()->GetClass(aTypeName));
}

bool Red::TweakDB::IsRecordType(const Red::CClass* aType)
{
    static Red::CBaseRTTIType* s_baseRecordType = Red::CRTTISystem::Get()->GetClass(BaseRecordTypeName);

    return aType && aType != s_baseRecordType && aType->IsA(s_baseRecordType);
}

bool Red::TweakDB::IsArrayType(Red::CName aTypeName)
{
    switch (aTypeName)
    {
    case EFlatType::Int32Array:
    case EFlatType::FloatArray:
    case EFlatType::BoolArray:
    case EFlatType::StringArray:
    case EFlatType::CNameArray:
    case EFlatType::TweakDBIDArray:
    case EFlatType::LocKeyArray:
    case EFlatType::ResourceArray:
    case EFlatType::QuaternionArray:
    case EFlatType::EulerAnglesArray:
    case EFlatType::Vector3Array:
    case EFlatType::Vector2Array:
    case EFlatType::ColorArray:
        return true;
    default:
        return false;
    }
}

bool Red::TweakDB::IsArrayType(const Red::CBaseRTTIType* aType)
{
    return aType && IsArrayType(aType->GetName());
}

bool Red::TweakDB::IsForeignKey(Red::CName aTypeName)
{
    return aTypeName == EFlatType::TweakDBID;
}

bool Red::TweakDB::IsForeignKey(const Red::CBaseRTTIType* aType)
{
    return aType && IsForeignKey(aType->GetName());
}

bool Red::TweakDB::IsForeignKeyArray(Red::CName aTypeName)
{
    return aTypeName == EFlatType::TweakDBIDArray;
}

bool Red::TweakDB::IsForeignKeyArray(const Red::CBaseRTTIType* aType)
{
    return aType && IsForeignKeyArray(aType->GetName());
}

bool Red::TweakDB::IsResRefToken(Red::CName aTypeName)
{
    return aTypeName == ResRefTokenTypeName;
}

bool Red::TweakDB::IsResRefToken(const Red::CBaseRTTIType* aType)
{
    return aType && IsResRefToken(aType->GetName());
}

bool Red::TweakDB::IsResRefTokenArray(Red::CName aTypeName)
{
    return aTypeName == ResRefTokenArrayTypeName;
}

bool Red::TweakDB::IsResRefTokenArray(const Red::CBaseRTTIType* aType)
{
    return aType && IsResRefTokenArray(aType->GetName());
}

Red::CName Red::TweakDB::GetArrayType(Red::CName aTypeName)
{
    switch (aTypeName)
    {
    case EFlatType::Int32: return EFlatType::Int32Array;
    case EFlatType::Float: return EFlatType::FloatArray;
    case EFlatType::Bool: return EFlatType::BoolArray;
    case EFlatType::String: return EFlatType::StringArray;
    case EFlatType::CName: return EFlatType::CNameArray;
    case EFlatType::TweakDBID: return EFlatType::TweakDBIDArray;
    case EFlatType::LocKey: return EFlatType::LocKeyArray;
    case EFlatType::Resource: return EFlatType::ResourceArray;
    case EFlatType::Quaternion: return EFlatType::QuaternionArray;
    case EFlatType::EulerAngles: return EFlatType::EulerAnglesArray;
    case EFlatType::Vector3: return EFlatType::Vector3Array;
    case EFlatType::Vector2: return EFlatType::Vector2Array;
    case EFlatType::Color: return EFlatType::ColorArray;
    }

    return {};
}

Red::CName Red::TweakDB::GetArrayType(const Red::CBaseRTTIType* aType)
{
    if (!aType)
        return {};

    return GetArrayType(aType->GetName());
}

Red::CName Red::TweakDB::GetElementType(Red::CName aTypeName)
{
    switch (aTypeName)
    {
    case EFlatType::Int32Array: return EFlatType::Int32;
    case EFlatType::FloatArray: return EFlatType::Float;
    case EFlatType::BoolArray: return EFlatType::Bool;
    case EFlatType::StringArray: return EFlatType::String;
    case EFlatType::CNameArray: return EFlatType::CName;
    case EFlatType::TweakDBIDArray: return EFlatType::TweakDBID;
    case EFlatType::LocKeyArray: return EFlatType::LocKey;
    case EFlatType::ResourceArray: return EFlatType::Resource;
    case EFlatType::QuaternionArray: return EFlatType::Quaternion;
    case EFlatType::EulerAnglesArray: return EFlatType::EulerAngles;
    case EFlatType::Vector3Array: return EFlatType::Vector3;
    case EFlatType::Vector2Array: return EFlatType::Vector2;
    case EFlatType::ColorArray: return EFlatType::Color;
    }

    return {};
}

Red::CName Red::TweakDB::GetElementType(const Red::CBaseRTTIType* aType)
{
    if (!aType)
        return {};

    return GetElementType(aType->GetName());
}

Red::CName Red::TweakDB::GetRecordFullName(Red::CName aName)
{
    return GetRecordFullName(aName.ToString());
}

Red::CName Red::TweakDB::GetRecordFullName(const char* aName)
{
    std::string finalName = aName;

    if (!finalName.starts_with(RecordTypePrefix))
        finalName.insert(0, RecordTypePrefix);

    if (!finalName.ends_with(RecordTypeSuffix))
        finalName.append(RecordTypeSuffix);

    return finalName.c_str();
}

std::string Red::TweakDB::GetRecordShortName(Red::CName aName)
{
    return GetRecordShortName(aName.ToString());
}

std::string Red::TweakDB::GetRecordShortName(const char* aName)
{
    std::string finalName = aName;

    if (finalName.starts_with(RecordTypePrefix))
        finalName.erase(0, RecordTypePrefixLength);

    if (finalName.ends_with(RecordTypeSuffix))
        finalName.erase(finalName.end() - RecordTypeSuffixLength, finalName.end());

    return finalName;
}

Red::TweakDBID Red::TweakDB::GetDefaultValueID(Red::CName aTypeName, const std::string& aPropName)
{
    std::string flatName = DefaultsGroup;

    flatName.append(GetRecordShortName(aTypeName));

    if (!aPropName.starts_with(PropSeparator))
    {
        flatName.append(PropSeparator);
    }

    flatName.append(aPropName);

    return {flatName.c_str()};
}

Core::SharedPtr<void> Red::TweakDB::MakeDefaultValue(Red::CName aTypeName)
{
    switch (aTypeName)
    {
    case EFlatType::Int32: return Core::MakeShared<int>();
    case EFlatType::Float: return Core::MakeShared<float>();
    case EFlatType::Bool: return Core::MakeShared<bool>();
    case EFlatType::String: return Core::MakeShared<Red::CString>();
    case EFlatType::CName: return Core::MakeShared<Red::CName>();
    case EFlatType::TweakDBID: return Core::MakeShared<Red::TweakDBID>();
    case EFlatType::LocKey: return Core::MakeShared<Red::gamedataLocKeyWrapper>();
    case EFlatType::Resource: return Core::MakeShared<Red::ResourceAsyncReference<>>();
    case EFlatType::Quaternion: return Core::MakeShared<Red::Quaternion>();
    case EFlatType::EulerAngles: return Core::MakeShared<Red::EulerAngles>();
    case EFlatType::Vector3: return Core::MakeShared<Red::Vector3>();
    case EFlatType::Vector2: return Core::MakeShared<Red::Vector2>();
    case EFlatType::Color: return Core::MakeShared<Red::Color>();
    case EFlatType::Int32Array: return Core::MakeShared<Red::DynArray<int>>();
    case EFlatType::FloatArray: return Core::MakeShared<Red::DynArray<float>>();
    case EFlatType::BoolArray: return Core::MakeShared<Red::DynArray<bool>>();
    case EFlatType::StringArray: return Core::MakeShared<Red::DynArray<Red::CString>>();
    case EFlatType::CNameArray: return Core::MakeShared<Red::DynArray<Red::CName>>();
    case EFlatType::TweakDBIDArray: return Core::MakeShared<Red::DynArray<Red::TweakDBID>>();
    case EFlatType::LocKeyArray: return Core::MakeShared<Red::DynArray<Red::gamedataLocKeyWrapper>>();
    case EFlatType::ResourceArray: return Core::MakeShared<Red::DynArray<Red::ResourceAsyncReference<>>>();
    case EFlatType::QuaternionArray: return Core::MakeShared<Red::DynArray<Red::Quaternion>>();
    case EFlatType::EulerAnglesArray: return Core::MakeShared<Red::DynArray<Red::EulerAngles>>();
    case EFlatType::Vector3Array: return Core::MakeShared<Red::DynArray<Red::Vector3>>();
    case EFlatType::Vector2Array: return Core::MakeShared<Red::DynArray<Red::Vector2>>();
    case EFlatType::ColorArray: return Core::MakeShared<Red::DynArray<Red::Color>>();
    }

    return nullptr;
}

Core::SharedPtr<void> Red::TweakDB::MakeDefaultValue(const Red::CBaseRTTIType* aType)
{
    if (!aType)
        return {};

    return MakeDefaultValue(aType->GetName());
}

