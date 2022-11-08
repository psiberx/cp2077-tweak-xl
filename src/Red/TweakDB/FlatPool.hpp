#pragma once

#include "Alias.hpp"

#include <RED4ext/TweakDB.hpp>

namespace Red::TweakDB
{
class FlatPool
{
public:
    static constexpr int32_t InvalidOffset = -1;

    struct Stats
    {
        float initTime = 0.0; // ms
        float updateTime = 0.0; // ms
        size_t poolSize = 0; // bytes
        size_t poolValues = 0;
        size_t knownTypes = 0;
        size_t flatEntries = 0;
    };

    FlatPool();
    explicit FlatPool(Instance* aTweakDb);

    int32_t AllocateData(const Red::CStackType& aData);
    int32_t AllocateValue(const Red::CBaseRTTIType* aType, Red::ScriptInstance aValue);
    int32_t AllocateDefault(const Red::CBaseRTTIType* aType);

    Red::CStackType GetData(int32_t aOffset);
    Red::ScriptInstance GetValuePtr(int32_t aOffset);

    [[nodiscard]] Stats GetStats() const;

private:
    struct FlatTypeInfo
    {
        Red::CBaseRTTIType* type;
        uintptr_t offset;
    };

    using FlatValueMap = Core::Map<uint64_t, int32_t>; // Hash -> Offset
    using FlatPoolMap = Core::Map<Red::CName, FlatValueMap>; // TypeName -> Pool
    using FlatDefaultMap = Core::Map<Red::CName, int32_t>; // TypeName -> Offset
    using FlatTypeMap = Core::Map<uintptr_t, FlatTypeInfo>; // VFT -> TypeInfo

    void Initialize();
    void UpdateStats(float updateTime = 0);
    void SyncBuffer();

    inline Red::CStackType GetFlatData(int32_t aOffset);
    inline static uint64_t Hash(const Red::CBaseRTTIType* aType, Red::ScriptInstance aValue);

    Instance* m_tweakDb;
    uintptr_t m_bufferEnd;
    uintptr_t m_offsetEnd;
    FlatPoolMap m_pools;
    FlatDefaultMap m_defaults;
    FlatTypeMap m_vfts;
    Stats m_stats;
};
}
