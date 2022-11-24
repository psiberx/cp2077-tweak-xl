#pragma once

#include "Red/TweakDB/Alias.hpp"
#include "Red/TweakDB/Buffer.hpp"
#include "Red/TweakDB/Reflection.hpp"

namespace Red
{
class TweakDBManager
{
public:
    class Batch
    {
        Red::SortedUniqueArray<Red::TweakDBID> flats;
        Core::Map<Red::TweakDBID, const Red::TweakDBRecordInfo*> records;
        Core::Map<Red::TweakDBID, const std::string> names;
        std::shared_mutex mutex;
        friend TweakDBManager;
    };

    using BatchPtr = Core::SharedPtr<Batch>;

    TweakDBManager();
    explicit TweakDBManager(Red::TweakDB* aTweakDb);
    explicit TweakDBManager(Core::SharedPtr<Red::TweakDBReflection> aReflection);

    TweakDBManager(const TweakDBManager&) = delete;
    TweakDBManager& operator=(const TweakDBManager&) = delete;

    Red::Value<> GetFlat(Red::TweakDBID aFlatId);
    Red::Value<> GetDefault(const Red::CBaseRTTIType* aType);
    Red::Handle<Red::TweakDBRecord> GetRecord(Red::TweakDBID aRecordId);
    Red::CClass* GetRecordType(Red::TweakDBID aRecordId);
    bool IsFlatExists(Red::TweakDBID aFlatId);
    bool IsRecordExists(Red::TweakDBID aRecordId);
    bool SetFlat(Red::TweakDBID aFlatId, const Red::CBaseRTTIType* aType, Red::Instance aInstance);
    bool SetFlat(Red::TweakDBID aFlatId, const Red::Value<>& aData);
    bool CreateRecord(Red::TweakDBID aRecordId, const Red::CClass* aType);
    bool CloneRecord(Red::TweakDBID aRecordId, Red::TweakDBID aSourceId);
    bool UpdateRecord(Red::TweakDBID aRecordId);
    void RegisterName(Red::TweakDBID aId, const std::string& aName);

    BatchPtr StartBatch();
    const Red::SortedUniqueArray<Red::TweakDBID>& GetFlats(const BatchPtr& aBatch);
    Red::Value<> GetFlat(const Red::TweakDBManager::BatchPtr& aBatch, Red::TweakDBID aFlatId);
    bool HasFlat(const BatchPtr& aBatch, Red::TweakDBID aFlatId);
    bool HasRecord(const BatchPtr& aBatch, Red::TweakDBID aRecordId);
    bool SetFlat(const BatchPtr& aBatch, Red::TweakDBID aFlatId, const Red::CBaseRTTIType* aType, Red::Instance aValue);
    bool SetFlat(const BatchPtr& aBatch, Red::TweakDBID aFlatId, const Red::Value<>& aData);
    bool CreateRecord(const BatchPtr& aBatch, Red::TweakDBID aRecordId, const Red::CClass* aType);
    bool CloneRecord(const BatchPtr& aBatch, Red::TweakDBID aRecordId, Red::TweakDBID aSourceId);
    bool UpdateRecord(const BatchPtr& aBatch, Red::TweakDBID aRecordId);
    void RegisterName(const BatchPtr& aBatch, Red::TweakDBID aId, const std::string& aName);
    void CommitBatch(const BatchPtr& aBatch);

    void Invalidate();

    Red::TweakDB* GetTweakDB();
    Core::SharedPtr<Red::TweakDBReflection>& GetReflection();

private:
    template<class SharedLockable>
    inline bool AssignFlat(Red::SortedUniqueArray<Red::TweakDBID>& aFlats, Red::TweakDBID aFlatId,
                           const Red::CBaseRTTIType* aType, Red::Instance aInstance,
                           SharedLockable& aMutex);
    inline void InheritFlats(Red::SortedUniqueArray<Red::TweakDBID>& aFlats, Red::TweakDBID aRecordId,
                             const Red::TweakDBRecordInfo* aRecordInfo);
    inline void InheritFlats(Red::SortedUniqueArray<Red::TweakDBID>& aFlats, Red::TweakDBID aRecordId,
                             const Red::TweakDBRecordInfo* aRecordInfo, Red::TweakDBID aSourceId);

    void CreateBaseName(Red::TweakDBID aId, const std::string& aName);
    void CreateExtraNames(Red::TweakDBID aId, const std::string& aName);

    Red::TweakDB* m_tweakDb;
    Core::SharedPtr<Red::TweakDBBuffer> m_buffer;
    Core::SharedPtr<Red::TweakDBReflection> m_reflection;
};
}
