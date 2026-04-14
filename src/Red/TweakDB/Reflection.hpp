#pragma once

#include "Red/TweakDB/RecordInfo.hpp"

namespace Red
{
constexpr auto RecordTypePrefix = "gamedata";
constexpr auto RecordTypePrefixLength = std::char_traits<char>::length(RecordTypePrefix);
constexpr auto RecordTypeSuffix = "_Record";
constexpr auto RecordTypeSuffixLength = std::char_traits<char>::length(RecordTypeSuffix);

template<typename T>
concept CStringConstructible =
    !std::is_pointer_v<T> && !std::is_reference_v<T> && std::is_constructible_v<T, const char*>;

namespace ERTDBFlatType
{
    enum : uint64_t
    {
        Int = Red::FNV1a64("Int32"),
        Float = Red::FNV1a64("Float"),
        Bool = Red::FNV1a64("Bool"),
        String = Red::FNV1a64("String"),
        CName = Red::FNV1a64("Red::CName"),
        LocKey = Red::FNV1a64("gamedataLocKeyWrapper"),
        ResRef = Red::FNV1a64("raRef:CResource"),
        TweakDBID = Red::FNV1a64("Red::TweakDBID"),
        Quaternion = Red::FNV1a64("Quaternion"),
        EulerAngles = Red::FNV1a64("EulerAngles"),
        Vector3 = Red::FNV1a64("Vector3"),
        Vector2 = Red::FNV1a64("Vector2"),
        Color = Red::FNV1a64("Color"),
        IntArray = Red::FNV1a64("array:Int32"),
        FloatArray = Red::FNV1a64("array:Float"),
        BoolArray = Red::FNV1a64("array:Bool"),
        StringArray = Red::FNV1a64("array:String"),
        CNameArray = Red::FNV1a64("array:Red::CName"),
        LocKeyArray = Red::FNV1a64("array:gamedataLocKeyWrapper"),
        ResRefArray = Red::FNV1a64("array:raRef:CResource"),
        TweakDBIDArray = Red::FNV1a64("array:Red::TweakDBID"),
        QuaternionArray = Red::FNV1a64("array:Quaternion"),
        EulerAnglesArray = Red::FNV1a64("array:EulerAngles"),
        Vector3Array = Red::FNV1a64("array:Vector3"),
        Vector2Array = Red::FNV1a64("array:Vector2"),
        ColorArray = Red::FNV1a64("array:Color"),
    };
} // namespace ERTDBFlatType

class TweakDBReflection
{
public:
    static const Red::CBaseRTTIType* GetFlatType(Red::CName aTypeName);
    static const Red::CClass* GetRecordType(Red::CName aTypeName);
    static const Red::CClass* GetRecordType(const char* aTypeName);

    static Red::CBaseRTTIType* GetArrayType(Red::CName aTypeName);
    static Red::CBaseRTTIType* GetArrayType(const Red::CBaseRTTIType* aType);

    static Red::CBaseRTTIType* GetElementType(Red::CName aTypeName);
    static Red::CBaseRTTIType* GetElementType(const Red::CBaseRTTIType* aType);

    static bool IsFlatType(Red::CName aTypeName);
    static bool IsFlatType(const Red::CBaseRTTIType* aType);

    static bool IsRecordType(Red::CName aTypeName);
    static bool IsRecordType(const Red::CClass* aType);

    static bool IsArrayType(Red::CName aTypeName);
    static bool IsArrayType(const Red::CBaseRTTIType* aType);

    static bool IsForeignKey(Red::CName aTypeName);
    static bool IsForeignKey(const Red::CBaseRTTIType* aType);

    static bool IsForeignKeyArray(Red::CName aTypeName);
    static bool IsForeignKeyArray(const Red::CBaseRTTIType* aType);

    static bool IsResRefToken(Red::CName aTypeName);
    static bool IsResRefToken(const Red::CBaseRTTIType* aType);

    static bool IsResRefTokenArray(Red::CName aTypeName);
    static bool IsResRefTokenArray(const Red::CBaseRTTIType* aType);

    static Red::CName GetArrayTypeName(Red::CName aTypeName);
    static Red::CName GetArrayTypeName(const Red::CBaseRTTIType* aType);

    static Red::CName GetElementTypeName(Red::CName aTypeName);
    static Red::CName GetElementTypeName(const Red::CBaseRTTIType* aType);

    template<CStringConstructible T>
    static T GetRecordFullName(Red::CName aName);
    template<CStringConstructible T>
    static T GetRecordFullName(const char* aName);

    template<CStringConstructible T>
    static T GetRecordAliasName(Red::CName aName);
    template<CStringConstructible T>
    static T GetRecordAliasName(const char* aName);

    template<CStringConstructible T>
    static T GetRecordShortName(Red::CName aName);
    template<CStringConstructible T>
    static T GetRecordShortName(const char* aName);

    static TweakDBRecordHash GetRecordTypeHash(const std::string& aName);

    static InstancePtr<> Construct(Red::CName aTypeName);
    static InstancePtr<> Construct(const Red::CBaseRTTIType* aType);

    static bool IsOriginalRecord(Red::TweakDBID aRecordId);
    static bool IsOriginalBaseRecord(Red::TweakDBID aParentId);
    static Red::TweakDBID GetOriginalParent(Red::TweakDBID aRecordId);
    static const Core::Set<Red::TweakDBID>& GetOriginalDescendants(Red::TweakDBID aSourceId);

    static void RegisterExtraFlat(Red::CName aRecordType, const std::string& aPropName, Red::CName aPropType,
                                  Red::CName aForeignType);
    static void RegisterDescendants(Red::TweakDBID aParentId, const Core::Set<Red::TweakDBID>& aDescendantIds);

    static std::string ToString(Red::TweakDBID aID);

    static IRTTISystem* GetRTTI();

    explicit TweakDBReflection(Red::TweakDB* aTweakDb);

    const Red::TweakDBRecordInfo* GetRecordInfo(Red::CName aTypeName);
    const Red::TweakDBRecordInfo* GetRecordInfo(const Red::CClass* aType);
    const Red::TweakDBRecordInfo* FindRecordInfo(Red::CName aTypeName);
    const Red::TweakDBRecordInfo* FindRecordInfo(const Red::CClass* aType);
    Red::TweakDB* GetTweakDB();
    Core::SharedPtr<const Red::TweakDBRecordInfo> RegisterRecordInfo(
        const Core::SharedPtr<Red::TweakDBRecordInfo>& aRecordInfo);

private:
    struct ExtraFlat
    {
        Red::CName typeName;
        Red::CName foreignTypeName;
        std::string appendix;
    };

    using ParentMap = Core::Map<Red::TweakDBID, Red::TweakDBID>;
    using DescendantMap = Core::Map<Red::TweakDBID, Core::Set<Red::TweakDBID>>;
    using ExtraFlatMap = Core::Map<Red::CName, Core::Vector<ExtraFlat>>;
    using RecordInfoMap = Core::Map<Red::CName, Core::SharedPtr<const Red::TweakDBRecordInfo>>;

    const Red::TweakDBRecordInfo* CollectRecordInfo(const Red::CClass* aType, Red::TweakDBID aSampleId = {});
    Red::TweakDBID GetRecordSampleId(const Red::CClass* aType);
    std::string ResolvePropertyName(Red::TweakDBID aSampleId, Red::CName aGetterName);
    int32_t ResolveDefaultValue(const Red::CClass* aType, const std::string& aPropName);

    Red::TweakDB* m_tweakDb;
    RecordInfoMap m_resolved;
    std::shared_mutex m_mutex;

    inline static IRTTISystem* s_rtti;
    inline static ParentMap s_parentMap;
    inline static DescendantMap s_descendantMap;
    inline static ExtraFlatMap s_extraFlats;
};

template<CStringConstructible T>
T TweakDBReflection::GetRecordFullName(Red::CName aName)
{
    return GetRecordFullName<T>(aName.ToString());
}

template<CStringConstructible T>
T TweakDBReflection::GetRecordFullName(const char* aName)
{
    std::string finalName = aName;

    if (finalName.empty())
        return {};

    if (!finalName.starts_with(RecordTypePrefix))
        finalName.insert(0, RecordTypePrefix);

    if (!finalName.ends_with(RecordTypeSuffix))
        finalName.append(RecordTypeSuffix);

    if constexpr (std::is_same_v<std::decay_t<T>, Red::CName>)
        return Red::CNamePool::Add(finalName.c_str());
    return T(finalName.c_str());
}

template<CStringConstructible T>
T TweakDBReflection::GetRecordAliasName(Red::CName aName)
{
    return GetRecordAliasName<T>(aName.ToString());
}

template<CStringConstructible T>
T TweakDBReflection::GetRecordAliasName(const char* aName)
{
    std::string finalName = aName;

    if (finalName.starts_with(RecordTypePrefix))
    {
        finalName.erase(0, RecordTypePrefixLength);
    }

    if (!finalName.ends_with(RecordTypeSuffix))
    {
        finalName.append(RecordTypeSuffix);
    }

    if constexpr (std::is_same_v<std::decay_t<T>, Red::CName>)
        return Red::CNamePool::Add(finalName.c_str());
    return T(finalName.c_str());
}

template<CStringConstructible T>
T TweakDBReflection::GetRecordShortName(Red::CName aName)
{
    return GetRecordShortName<T>(aName.ToString());
}

template<CStringConstructible T>
T TweakDBReflection::GetRecordShortName(const char* aName)
{
    std::string finalName = aName;

    if (finalName.starts_with(RecordTypePrefix))
        finalName.erase(0, RecordTypePrefixLength);

    if (finalName.ends_with(RecordTypeSuffix))
        finalName.erase(finalName.end() - RecordTypeSuffixLength, finalName.end());

    if constexpr (std::is_same_v<std::decay_t<T>, Red::CName>)
        return Red::CNamePool::Add(finalName.c_str());
    return T(finalName.c_str());
}

} // namespace Red
