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
        Int = FNV1a64("Int32"),
        Float = FNV1a64("Float"),
        Bool = FNV1a64("Bool"),
        String = FNV1a64("String"),
        CName = FNV1a64("CName"),
        LocKey = FNV1a64("gamedataLocKeyWrapper"),
        ResRef = FNV1a64("raRef:CResource"),
        TweakDBID = FNV1a64("TweakDBID"),
        Quaternion = FNV1a64("Quaternion"),
        EulerAngles = FNV1a64("EulerAngles"),
        Vector3 = FNV1a64("Vector3"),
        Vector2 = FNV1a64("Vector2"),
        Color = FNV1a64("Color"),
        IntArray = FNV1a64("array:Int32"),
        FloatArray = FNV1a64("array:Float"),
        BoolArray = FNV1a64("array:Bool"),
        StringArray = FNV1a64("array:String"),
        CNameArray = FNV1a64("array:CName"),
        LocKeyArray = FNV1a64("array:gamedataLocKeyWrapper"),
        ResRefArray = FNV1a64("array:raRef:CResource"),
        TweakDBIDArray = FNV1a64("array:TweakDBID"),
        QuaternionArray = FNV1a64("array:Quaternion"),
        EulerAnglesArray = FNV1a64("array:EulerAngles"),
        Vector3Array = FNV1a64("array:Vector3"),
        Vector2Array = FNV1a64("array:Vector2"),
        ColorArray = FNV1a64("array:Color"),
    };
} // namespace ERTDBFlatType

class TweakDBReflection
{
public:
    static const CBaseRTTIType* GetFlatType(const CName& aTypeName);
    static const CClass* GetRecordType(const CName& aTypeName);
    static const CClass* GetRecordType(const char* aTypeName);

    static CBaseRTTIType* GetArrayType(const CName& aTypeName);
    static CBaseRTTIType* GetArrayType(const CBaseRTTIType* aType);

    static CBaseRTTIType* GetElementType(const CName& aTypeName);
    static CBaseRTTIType* GetElementType(const CBaseRTTIType* aType);

    static bool IsFlatType(const CName& aTypeName);
    static bool IsFlatType(const CBaseRTTIType* aType);

    static bool IsRecordType(const CName& aTypeName);
    static bool IsRecordType(const CClass* aType);

    static bool IsArrayType(const CName& aTypeName);
    static bool IsArrayType(const CBaseRTTIType* aType);

    static bool IsForeignKey(const CName& aTypeName);
    static bool IsForeignKey(const CBaseRTTIType* aType);

    static bool IsForeignKeyArray(const CName& aTypeName);
    static bool IsForeignKeyArray(const CBaseRTTIType* aType);

    static bool IsResRefToken(const CName& aTypeName);
    static bool IsResRefToken(const CBaseRTTIType* aType);

    static bool IsResRefTokenArray(const CName& aTypeName);
    static bool IsResRefTokenArray(const CBaseRTTIType* aType);

    static CName GetArrayTypeName(const CName& aTypeName);
    static CName GetArrayTypeName(const CBaseRTTIType* aType);

    static CName GetElementTypeName(const CName& aTypeName);
    static CName GetElementTypeName(const CBaseRTTIType* aType);

    template<CStringConstructible T>
    static T GetRecordFullName(const CName& aName);
    template<CStringConstructible T>
    static T GetRecordFullName(const char* aName);

    template<CStringConstructible T>
    static T GetRecordAliasName(const CName& aName);
    template<CStringConstructible T>
    static T GetRecordAliasName(const char* aName);

    template<CStringConstructible T>
    static T GetRecordShortName(const CName& aName);
    template<CStringConstructible T>
    static T GetRecordShortName(const char* aName);

    static uint32_t GetRecordTypeHash(const std::string& aName);

    static InstancePtr<> Construct(const CName& aTypeName);
    static InstancePtr<> Construct(const CBaseRTTIType* aType);

    static bool IsOriginalRecord(TweakDBID aRecordId);
    static bool IsOriginalBaseRecord(TweakDBID aParentId);
    static TweakDBID GetOriginalParent(TweakDBID aRecordId);
    static const Core::Set<TweakDBID>& GetOriginalDescendants(TweakDBID aSourceId);

    static void RegisterExtraFlat(const CName& aRecordType, const std::string& aPropName, const CName& aPropType,
                                  const CName& aForeignType);
    static void RegisterDescendants(TweakDBID aParentId, const Core::Set<TweakDBID>& aDescendantIds);

    static std::string ToString(TweakDBID aID);

    static IRTTISystem* GetRTTI();

    explicit TweakDBReflection(TweakDB* aTweakDb);

    const TweakDBRecordInfo* GetRecordInfo(const CName& aTypeName);
    const TweakDBRecordInfo* GetRecordInfo(const CClass* aType);
    const TweakDBRecordInfo* FindRecordInfo(const CName& aTypeName) const;
    const TweakDBRecordInfo* FindRecordInfo(const CClass* aType) const;
    TweakDB* GetTweakDB() const;
    Core::SharedPtr<const TweakDBRecordInfo> RegisterRecordInfo(const Core::SharedPtr<TweakDBRecordInfo>& aRecordInfo);

private:
    struct ExtraFlat
    {
        CName typeName;
        CName foreignTypeName;
        std::string appendix;
    };

    using ParentMap = Core::Map<TweakDBID, TweakDBID>;
    using DescendantMap = Core::Map<TweakDBID, Core::Set<TweakDBID>>;
    using ExtraFlatMap = Core::Map<CName, Core::Vector<ExtraFlat>>;
    using RecordInfoMap = Core::Map<CName, Core::SharedPtr<const TweakDBRecordInfo>>;

    const TweakDBRecordInfo* CollectRecordInfo(const CClass* aType, TweakDBID aSampleId = {});
    TweakDBID GetRecordSampleId(const CClass* aType) const;
    std::string ResolvePropertyName(TweakDBID aSampleId, const CName& aGetterName) const;
    int32_t ResolveDefaultValue(const CClass* aType, const std::string& aPropName) const;

    TweakDB* m_tweakDb;
    RecordInfoMap m_resolved;
    mutable std::shared_mutex m_mutex;

    inline static IRTTISystem* s_rtti;
    inline static ParentMap s_parentMap;
    inline static DescendantMap s_descendantMap;
    inline static ExtraFlatMap s_extraFlats;
};

template<CStringConstructible T>
T TweakDBReflection::GetRecordFullName(const CName& aName)
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

    if constexpr (std::is_same_v<std::decay_t<T>, CName>)
        return CNamePool::Add(finalName.c_str());
    return T(finalName.c_str());
}

template<CStringConstructible T>
T TweakDBReflection::GetRecordAliasName(const CName& aName)
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

    if constexpr (std::is_same_v<std::decay_t<T>, CName>)
        return CNamePool::Add(finalName.c_str());
    return T(finalName.c_str());
}

template<CStringConstructible T>
T TweakDBReflection::GetRecordShortName(const CName& aName)
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

    if constexpr (std::is_same_v<std::decay_t<T>, CName>)
        return CNamePool::Add(finalName.c_str());
    return T(finalName.c_str());
}

} // namespace Red
