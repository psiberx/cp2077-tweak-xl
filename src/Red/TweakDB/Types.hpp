#pragma once

#include "Alias.hpp"

#include <RED4ext/CName.hpp>
#include <RED4ext/RTTITypes.hpp>

namespace Red::TweakDB
{
namespace EFlatType
{
enum : uint64_t
{
    Int32 = Red::FNV1a64("Int32"),
    Float = Red::FNV1a64("Float"),
    Bool = Red::FNV1a64("Bool"),
    String = Red::FNV1a64("String"),
    CName = Red::FNV1a64("CName"),
    TweakDBID = Red::FNV1a64("TweakDBID"),
    LocKey = Red::FNV1a64("gamedataLocKeyWrapper"),
    Resource = Red::FNV1a64("raRef:CResource"),
    Quaternion = Red::FNV1a64("Quaternion"),
    EulerAngles = Red::FNV1a64("EulerAngles"),
    Vector3 = Red::FNV1a64("Vector3"),
    Vector2 = Red::FNV1a64("Vector2"),
    Color = Red::FNV1a64("Color"),
    Int32Array = Red::FNV1a64("array:Int32"),
    FloatArray = Red::FNV1a64("array:Float"),
    BoolArray = Red::FNV1a64("array:Bool"),
    StringArray = Red::FNV1a64("array:String"),
    CNameArray = Red::FNV1a64("array:CName"),
    TweakDBIDArray = Red::FNV1a64("array:TweakDBID"),
    LocKeyArray = Red::FNV1a64("array:gamedataLocKeyWrapper"),
    ResourceArray = Red::FNV1a64("array:raRef:CResource"),
    QuaternionArray = Red::FNV1a64("array:Quaternion"),
    EulerAnglesArray = Red::FNV1a64("array:EulerAngles"),
    Vector3Array = Red::FNV1a64("array:Vector3"),
    Vector2Array = Red::FNV1a64("array:Vector2"),
    ColorArray = Red::FNV1a64("array:Color"),
};
}

bool IsFlatType(Red::CName aTypeName);
bool IsFlatType(const Red::CBaseRTTIType* aType);

bool IsRecordType(Red::CName aTypeName);
bool IsRecordType(const Red::CClass* aType);

bool IsArrayType(Red::CName aTypeName);
bool IsArrayType(const Red::CBaseRTTIType* aType);

bool IsForeignKey(Red::CName aTypeName);
bool IsForeignKey(const Red::CBaseRTTIType* aType);

bool IsForeignKeyArray(Red::CName aTypeName);
bool IsForeignKeyArray(const Red::CBaseRTTIType* aType);

bool IsResRefToken(Red::CName aTypeName);
bool IsResRefToken(const Red::CBaseRTTIType* aType);

bool IsResRefTokenArray(Red::CName aTypeName);
bool IsResRefTokenArray(const Red::CBaseRTTIType* aType);

Red::CName GetArrayType(Red::CName aTypeName);
Red::CName GetArrayType(const Red::CBaseRTTIType* aType);

Red::CName GetElementType(Red::CName aTypeName);
Red::CName GetElementType(const Red::CBaseRTTIType* aType);

Red::CName GetRecordFullName(Red::CName aName);
Red::CName GetRecordFullName(const char* aName);

std::string GetRecordShortName(Red::CName aName);
std::string GetRecordShortName(const char* aName);

Core::SharedPtr<void> MakeDefaultValue(Red::CName aTypeName);
Core::SharedPtr<void> MakeDefaultValue(const Red::CBaseRTTIType* aType);
}
