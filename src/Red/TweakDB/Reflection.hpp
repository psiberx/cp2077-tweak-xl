#pragma once

#include "Red/TweakDB/Alias.hpp"

namespace Red
{
namespace ETweakDBFlatType
{
enum : uint64_t
{
    Int = Red::FNV1a64("Int32"),
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
    IntArray = Red::FNV1a64("array:Int32"),
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

struct TweakDBPropertyInfo
{
    Red::CName name;
    const Red::CBaseRTTIType* type;
    const Red::CBaseRTTIType* elementType;
    const Red::CClass* foreignType;
    bool isArray;
    bool isForeignKey;
    std::string appendix; // The name used to build ID of the property
    uintptr_t offset; // Offset of the property in record instance
    int32_t defaultValue; // Offset of the value in the buffer todo: default
};

struct TweakDBRecordInfo
{
    Red::CName name;
    const Red::CClass* type;
    const Red::CClass* parent;
    Core::Map<Red::CName, Core::SharedPtr<Red::TweakDBPropertyInfo>> props;
    bool extraFlats;
    std::string shortName;
    uint32_t typeHash;

    [[nodiscard]] const Red::TweakDBPropertyInfo* GetPropInfo(Red::CName aPropName) const
    {
        const auto& propIt = props.find(aPropName);
        return propIt != props.end() ? propIt->second.get() : nullptr;
    }
};

class TweakDBReflection
{
public:
    TweakDBReflection();
    explicit TweakDBReflection(Red::TweakDB* aTweakDb);

    const Red::TweakDBRecordInfo* GetRecordInfo(Red::CName aTypeName);
    const Red::TweakDBRecordInfo* GetRecordInfo(const Red::CClass* aType);

    const Red::CBaseRTTIType* GetFlatType(Red::CName aTypeName);
    const Red::CClass* GetRecordType(Red::CName aTypeName);
    const Red::CClass* GetRecordType(const char* aTypeName);

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

    Core::SharedPtr<void> Construct(Red::CName aTypeName);
    Core::SharedPtr<void> Construct(const Red::CBaseRTTIType* aType);

    Core::SharedPtr<void> Clone(Red::CName aTypeName, void* aValue);
    Core::SharedPtr<void> Clone(const Red::CBaseRTTIType* aType, void* aValue);

    Red::TweakDB* GetTweakDB();

private:
    struct ExtraFlat
    {
        Red::CName typeName;
        Red::CName foreignTypeName;
        std::string appendix;
    };

    using ExtraFlatMap = Core::Map<Red::CName, Core::Vector<ExtraFlat>>;
    using RecordInfoMap = Core::Map<Red::CName, Core::SharedPtr<Red::TweakDBRecordInfo>>;

    Core::SharedPtr<Red::TweakDBRecordInfo> CollectRecordInfo(const Red::CClass* aType, Red::TweakDBID aSampleId = {});

    Red::TweakDBID GetRecordSampleId(const Red::CClass* aType);
    uint32_t GetRecordTypeHash(const Red::CClass* aType);
    std::string ResolvePropertyName(Red::TweakDBID aSampleId, Red::CName aGetterName);
    int32_t ResolveDefaultValue(const Red::CClass* aType, const std::string& aPropName);

    Red::TweakDB* m_tweakDb;
    Red::CRTTISystem* m_rtti;
    RecordInfoMap m_resolved;

    static ExtraFlatMap s_extraFlats;
};
}
