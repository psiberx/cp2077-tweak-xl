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

    TweakDBManager();
    explicit TweakDBManager(Red::TweakDB* aTweakDb);
    explicit TweakDBManager(Core::SharedPtr<Red::TweakDBReflection> aReflection);

    TweakDBManager(const TweakDBManager&) = delete;
    TweakDBManager& operator=(const TweakDBManager&) = delete;

    Red::CStackType GetFlat(Red::TweakDBID aFlatId);
    Red::Handle<Red::TweakDBRecord> GetRecord(Red::TweakDBID aRecordId);
    Red::CClass* GetRecordType(Red::TweakDBID aRecordId);
    bool IsFlatExists(Red::TweakDBID aFlatId);
    bool IsRecordExists(Red::TweakDBID aRecordId);
    bool SetFlat(Red::TweakDBID aFlatId, const Red::CBaseRTTIType* aType, Red::ScriptInstance aValue);
    bool SetFlat(Red::TweakDBID aFlatId, Red::CStackType aData);
    bool CreateRecord(Red::TweakDBID aRecordId, const Red::CClass* aType);
    bool CloneRecord(Red::TweakDBID aRecordId, Red::TweakDBID aSourceId);
    bool UpdateRecord(Red::TweakDBID aRecordId);
    void RegisterName(Red::TweakDBID aId, const std::string& aName);

    Core::SharedPtr<Batch> StartBatch();
    bool HasFlat(const Core::SharedPtr<Batch>& aBatch, Red::TweakDBID aFlatId);
    bool HasRecord(const Core::SharedPtr<Batch>& aBatch, Red::TweakDBID aRecordId);
    bool SetFlat(const Core::SharedPtr<Batch>& aBatch, Red::TweakDBID aFlatId,
                 const Red::CBaseRTTIType* aType, Red::ScriptInstance aValue);
    bool SetFlat(const Core::SharedPtr<Batch>& aBatch, Red::TweakDBID aFlatId, Red::CStackType aData);
    bool CreateRecord(const Core::SharedPtr<Batch>& aBatch, Red::TweakDBID aRecordId, const Red::CClass* aType);
    bool CloneRecord(const Core::SharedPtr<Batch>& aBatch, Red::TweakDBID aRecordId, Red::TweakDBID aSourceId);
    bool UpdateRecord(const Core::SharedPtr<Batch>& aBatch, Red::TweakDBID aRecordId);
    void RegisterName(const Core::SharedPtr<Batch>& aBatch, Red::TweakDBID aId, const std::string& aName);
    void CommitBatch(const Core::SharedPtr<Batch>& aBatch);

    void Invalidate();

    Red::TweakDB* GetTweakDB();
    Core::SharedPtr<Red::TweakDBReflection>& GetReflection();

private:
    template<class SharedLockable>
    inline bool AssignFlat(Red::SortedUniqueArray<Red::TweakDBID>& aFlats, Red::TweakDBID aFlatId,
                           const Red::CBaseRTTIType* aType, Red::ScriptInstance aValue,
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
