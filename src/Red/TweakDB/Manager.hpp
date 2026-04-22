#pragma once

#include <memory>

#include "Red/TweakDB/Alias.hpp"
#include "Red/TweakDB/Buffer.hpp"
#include "Red/TweakDB/Reflection.hpp"

namespace Red
{
class ScriptableTweakDBRecord;

class TweakDBManager
{
public:
    class Batch
    {
        Core::Set<Red::TweakDBID> flats;
        Core::Map<Red::TweakDBID, RecordInfo> records;
        Core::Map<Red::TweakDBID, const std::string> names;
        std::shared_mutex mutex;
        friend TweakDBManager;
    };

    using BatchPtr = Core::SharedPtr<Batch>;

    explicit TweakDBManager(Core::SharedPtr<Red::TweakDBReflection> aReflection);

    TweakDBManager(const TweakDBManager&) = delete;
    TweakDBManager& operator=(const TweakDBManager&) = delete;

    Red::Value<> GetFlat(Red::TweakDBID aFlatId);
    Red::Value<> GetDefault(const Red::CBaseRTTIType* aType);
    Red::Handle<Red::TweakDBRecord> GetRecord(Red::TweakDBID aRecordId);
    const Red::CClass* GetRecordType(Red::TweakDBID aRecordId);
    bool IsFlatExists(Red::TweakDBID aFlatId);
    bool IsRecordExists(Red::TweakDBID aRecordId);
    bool SetFlat(Red::TweakDBID aFlatId, const Red::CBaseRTTIType* aType, Red::Instance aInstance);
    bool SetFlat(Red::TweakDBID aFlatId, const Red::Value<>& aData);
    bool CreateRecord(Red::TweakDBID aRecordId, const Red::CClass* aType);
    bool CreateScriptableRecord(Red::TweakDB* aTweakDB, Red::TweakDBID aRecordId, uint32_t aHash);
    bool CloneRecord(Red::TweakDBID aRecordId, Red::TweakDBID aSourceId);
    bool InheritProps(Red::TweakDBID aRecordId, Red::TweakDBID aSourceId);
    bool UpdateRecord(Red::TweakDBID aRecordId);
    void RegisterEnum(Red::TweakDBID aRecordId);
    void RegisterName(const std::string& aName, const Red::CClass* aType = nullptr);
    void RegisterName(Red::TweakDBID aId, const std::string& aName, const Red::CClass* aType = nullptr);
    const Core::Set<Red::TweakDBID>& GetEnums();
    std::string_view GetName(Red::TweakDBID aId);
    BatchPtr StartBatch();
    const Core::Set<Red::TweakDBID>& GetFlats(const BatchPtr& aBatch);
    Red::Value<> GetFlat(const BatchPtr& aBatch, Red::TweakDBID aFlatId);
    const Red::CClass* GetRecordType(const BatchPtr& aBatch, Red::TweakDBID aRecordId);
    bool IsFlatExists(const BatchPtr& aBatch, Red::TweakDBID aFlatId);
    bool IsRecordExists(const BatchPtr& aBatch, Red::TweakDBID aRecordId);
    bool SetFlat(const BatchPtr& aBatch, Red::TweakDBID aFlatId, const Red::CBaseRTTIType* aType, Red::Instance aValue);
    bool SetFlat(const BatchPtr& aBatch, Red::TweakDBID aFlatId, const Red::Value<>& aData);
    bool CreateRecord(const BatchPtr& aBatch, Red::TweakDBID aRecordId, const Red::CClass* aType);
    bool CloneRecord(const BatchPtr& aBatch, Red::TweakDBID aRecordId, Red::TweakDBID aSourceId);
    bool InheritProps(const BatchPtr& aBatch, Red::TweakDBID aRecordId, Red::TweakDBID aSourceId);
    bool UpdateRecord(const BatchPtr& aBatch, Red::TweakDBID aRecordId);
    void RegisterEnum(const BatchPtr& aBatch, Red::TweakDBID aRecordId);
    void RegisterName(const BatchPtr& aBatch, Red::TweakDBID aId, const std::string& aName);
    void CommitBatch(const BatchPtr& aBatch);

    bool RegisterScriptableRecordType(const std::string& aName, Red::CClass* aParent = nullptr);
    bool RegisterScriptableProperty(const std::string& aName, const std::string& aPropertyName, uint64_t aFlatType,
                                    const Red::CClass* aForeignType = nullptr);

    void Invalidate();

    Red::TweakDB* GetTweakDB();
    Core::SharedPtr<Red::TweakDBReflection>& GetReflection();

private:
    template<class SharedLockable>
    inline bool AssignFlat(Red::SortedUniqueArray<Red::TweakDBID>& aFlats, Red::TweakDBID aFlatId,
                           const Red::CBaseRTTIType* aType, Red::Instance aInstance, SharedLockable& aMutex);
    inline void InheritFlats(Red::SortedUniqueArray<Red::TweakDBID>& aFlats, Red::TweakDBID aRecordId,
                             RecordInfo aRecordInfo);
    inline void InheritFlats(Red::SortedUniqueArray<Red::TweakDBID>& aFlats, Red::TweakDBID aRecordId,
                             RecordInfo aRecordInfo, Red::TweakDBID aSourceId);

    inline bool AssignFlat(const Red::TweakDBManager::BatchPtr& aBatch, Red::TweakDBID aFlatId,
                           const Red::Value<>& aValue);
    inline void InheritFlats(const Red::TweakDBManager::BatchPtr& aBatch, Red::TweakDBID aRecordId,
                             RecordInfo aRecordInfo);
    inline void InheritFlats(const Red::TweakDBManager::BatchPtr& aBatch, Red::TweakDBID aRecordId,
                             RecordInfo aRecordInfo, Red::TweakDBID aSourceId);

    void CreateBaseName(Red::TweakDBID aId, const std::string& aName);
    void CreateExtraNames(Red::TweakDBID aId, const std::string& aName, const Red::CClass* aType = nullptr);

    Red::TweakDB* m_tweakDb;
    Red::CRTTISystem* m_rtti;
    Core::SharedPtr<Red::TweakDBBuffer> m_buffer;
    Core::SharedPtr<Red::TweakDBReflection> m_reflection;
    Core::Map<Red::TweakDBID, std::string> m_knownNames;
    Core::Set<Red::TweakDBID> m_knownEnums;
    std::shared_mutex m_mutex;
};
} // namespace Red
