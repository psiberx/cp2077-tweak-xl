#pragma once

namespace Red
{
namespace ERTDBFlatType
{
    enum : uint64_t
    {
        Int = Red::FNV1a64("Int32"),
        Float = Red::FNV1a64("Float"),
        Bool = Red::FNV1a64("Bool"),
        String = Red::FNV1a64("String"),
        CName = Red::FNV1a64("CName"),
        LocKey = Red::FNV1a64("gamedataLocKeyWrapper"),
        ResRef = Red::FNV1a64("raRef:CResource"),
        TweakDBID = Red::FNV1a64("TweakDBID"),
        Quaternion = Red::FNV1a64("Quaternion"),
        EulerAngles = Red::FNV1a64("EulerAngles"),
        Vector3 = Red::FNV1a64("Vector3"),
        Vector2 = Red::FNV1a64("Vector2"),
        Color = Red::FNV1a64("Color"),
        IntArray = Red::FNV1a64("array:Int32"),
        FloatArray = Red::FNV1a64("array:Float"),
        BoolArray = Red::FNV1a64("array:Bool"),
        StringArray = Red::FNV1a64("array:String"),
        CNameArray = Red::FNV1a64("array:CName"),
        LocKeyArray = Red::FNV1a64("array:gamedataLocKeyWrapper"),
        ResRefArray = Red::FNV1a64("array:raRef:CResource"),
        TweakDBIDArray = Red::FNV1a64("array:TweakDBID"),
        QuaternionArray = Red::FNV1a64("array:Quaternion"),
        EulerAnglesArray = Red::FNV1a64("array:EulerAngles"),
        Vector3Array = Red::FNV1a64("array:Vector3"),
        Vector2Array = Red::FNV1a64("array:Vector2"),
        ColorArray = Red::FNV1a64("array:Color"),
    };
} // namespace ERTDBFlatType

struct TweakDBRecordInfo;
struct TweakDBPropertyInfo;

using RecordInfo = Core::SharedPtr<Red::TweakDBRecordInfo>;
using PropertyInfo = Core::SharedPtr<Red::TweakDBPropertyInfo>;

struct TweakDBPropertyInfo
{
    Red::CName name;
    Red::CName functionName;
    const Red::CBaseRTTIType* type;
    const Red::CBaseRTTIType* elementType;
    const Red::CClass* foreignType;
    bool isArray;
    bool isForeignKey;
    bool isExtra;
    std::string appendix;                // The name used to build ID of the property
    std::optional<int32_t> defaultValue; // Offset of the default value in the buffer
};

struct TweakDBRecordInfo
{
    Red::CName name;
    Red::CName aliasName;
    Red::CClass* type;
    Red::CClass* parent;
    Core::Map<Red::CName, PropertyInfo> props;
    bool isScriptable;
    std::string shortName;
    uint32_t typeHash;

    [[nodiscard]] const Red::TweakDBPropertyInfo* GetPropInfo(Red::CName aPropName) const
    {
        if (const auto it = props.find(aPropName); it != props.end())
        {
            return it->second.get();
        }
        return nullptr;
    }
};
} // namespace Red
