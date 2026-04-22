#pragma once

#include "Types.hpp"

namespace Red
{
class RecordClass;
class ScriptableRecordClosureRegistry;
class ScriptableTweakDBRecord;

class TweakDBReflection
{
public:
    TweakDBReflection();
    explicit TweakDBReflection(Red::TweakDB* aTweakDb);
    ~TweakDBReflection();

    Red::RecordInfo GetRecordInfo(const std::string& aTypeName, bool aCollect = true);
    Red::RecordInfo GetRecordInfo(Red::CName aTypeName, bool aCollect = true);
    Red::RecordInfo GetRecordInfo(Red::CClass* aType, bool aCollect = true);
    Red::RecordInfo GetRecordInfo(const Red::CClass* aType, bool aCollect = true);

    Red::RecordInfo GetRecordInfoByHash(uint32_t aHash);
    Red::RecordInfo GetScriptableRecordInfo(uint32_t aHash);

    const Red::CBaseRTTIType* GetFlatType(uint64_t aType);
    const Red::CBaseRTTIType* GetFlatType(Red::CName aTypeName);
    const Red::CClass* GetRecordType(Red::CName aTypeName);
    const Red::CClass* GetRecordType(const char* aTypeName);

    Red::CBaseRTTIType* GetArrayType(Red::CName aTypeName);
    Red::CBaseRTTIType* GetArrayType(const Red::CBaseRTTIType* aType);

    Red::CBaseRTTIType* GetElementType(Red::CName aTypeName);
    Red::CBaseRTTIType* GetElementType(const Red::CBaseRTTIType* aType);

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

    Red::CName GetArrayTypeName(Red::CName aTypeName);
    Red::CName GetArrayTypeName(const Red::CBaseRTTIType* aType);

    Red::CName GetElementTypeName(Red::CName aTypeName);
    Red::CName GetElementTypeName(const Red::CBaseRTTIType* aType);

    Red::CName GetRecordFullName(Red::CName aName, bool aRegister = false);
    Red::CName GetRecordFullName(const char* aName, bool aRegister = false);

    Red::CName GetRecordAliasName(Red::CName aName, bool aRegister = false);
    Red::CName GetRecordAliasName(const char* aName, bool aRegister = false);

    std::string GetRecordShortName(Red::CName aName);
    std::string GetRecordShortName(const char* aName);

    std::string GetPropertyFunctionName(Red::CName aName);

    uint32_t GetRecordTypeHash(const std::string& aName);

    Red::InstancePtr<> Construct(Red::CName aTypeName);
    Red::InstancePtr<> Construct(const Red::CBaseRTTIType* aType);

    bool IsOriginalRecord(Red::TweakDBID aRecordId);
    bool IsOriginalBaseRecord(Red::TweakDBID aParentId);
    Red::TweakDBID GetOriginalParent(Red::TweakDBID aRecordId);
    const Core::Set<Red::TweakDBID>& GetOriginalDescendants(Red::TweakDBID aSourceId);

    void RegisterExtraFlat(Red::CName aRecordType, const std::string& aPropName, Red::CName aPropType,
                           Red::CName aForeignType);
    void RegisterDescendants(Red::TweakDBID aParentId, const Core::Set<Red::TweakDBID>& aDescendantIds);

    RecordInfo RegisterScriptableRecordType(const std::string& aName, Red::CClass* aParent = nullptr);
    PropertyInfo RegisterScriptableProperty(const RecordInfo& aRecord, const std::string& aPropertyName,
                                            uint64_t aFlatType, const Red::CClass* aForeignType = nullptr);
    Handle<ScriptableTweakDBRecord> ConstructScriptableRecord(Red::TweakDBID aRecordId, uint32_t aHash);
    void* GetScriptablePropertyValue(const Red::ScriptableTweakDBRecord* aRecord, const PropertyInfo& aPropertyInfo);

    std::string ToString(Red::TweakDBID aID);

    Red::TweakDB* GetTweakDB();

    Red::TweakDBID BuildRTDBID(Red::CName aRecordName, Red::CName aPropertyName);

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
    using RecordInfoByNameMap = Core::Map<Red::CName, RecordInfo>;
    using RecordInfoByHashMap = Core::Map<uint32_t, RecordInfo>;

    Red::RecordInfo CollectRecordInfo(Red::CClass* aType, Red::TweakDBID aSampleId = {});
    Red::TweakDBID GetRecordSampleId(const Red::CClass* aType);
    uint32_t GetRecordTypeHash(const Red::CClass* aType);
    std::string ResolvePropertyName(Red::TweakDBID aSampleId, Red::CName aGetterName);
    std::optional<int32_t> ResolveDefaultValue(const Red::CClass* aType, const std::string& aPropName);

    bool IsValid(const PropertyInfo& aPropInfo);
    bool IsValid(const RecordInfo& aRecordInfo);

    bool RegisterRecordInfo(RecordInfo aRecordInfo);
    bool RegisterPropertyInfo(const RecordInfo& aRecordInfo, const PropertyInfo& aPropertyInfo);
    void InheritRecordInfo(const RecordInfo&, Red::RecordInfo aParentInfo);
    RecordInfo CreateRecordInfo(Red::CName aName);
    RecordInfo CreateRecordInfo(Red::CClass* aClass);
    PropertyInfo CreatePropertyInfo(const std::string& aName, uint64_t aFlatType);
    PropertyInfo CreatePropertyInfo(const std::string& aName, const Red::CBaseRTTIType* aFlatType);
    Red::RecordClass* GetRecordClass(Red::CName aName);
    Red::RecordClass* CreateRecordClass(Red::CName aName, Red::CName aAliasName, uint32_t aHash);

    Red::TweakDB* m_tweakDb;
    Red::CRTTISystem* m_rtti;

    RecordInfoByNameMap m_recordInfoByName;
    RecordInfoByNameMap m_recordInfoByHash;
    std::shared_mutex m_mutex;

    Core::UniquePtr<Red::ScriptableRecordClosureRegistry> m_scriptableRecordClosures;
    Core::Map<Red::CName, Core::SharedPtr<Red::RecordClass>> m_scriptableRecordClasses;
    std::shared_mutex m_recordClassMutex;

    inline static ParentMap s_parentMap;
    inline static DescendantMap s_descendantMap;
    inline static ExtraFlatMap s_extraFlats;
};
} // namespace Red
