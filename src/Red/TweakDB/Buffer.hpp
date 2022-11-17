#pragma once

#include "Red/TweakDB/Alias.hpp"

namespace Red
{
class TweakDBBuffer
{
public:
    static constexpr int32_t InvalidOffset = -1;

    struct BufferStats
    {
        float initTime = 0.0; // ms
        float updateTime = 0.0; // ms
        size_t poolSize = 0; // bytes
        size_t poolValues = 0;
        size_t knownTypes = 0;
        size_t flatEntries = 0;
    };

    TweakDBBuffer();
    explicit TweakDBBuffer(Red::TweakDB* aTweakDb);

    int32_t AllocateData(const Red::CStackType& aData);
    int32_t AllocateValue(const Red::CBaseRTTIType* aType, Red::ScriptInstance aValue);
    int32_t AllocateDefault(const Red::CBaseRTTIType* aType);

    Red::CStackType GetData(int32_t aOffset);
    Red::ScriptInstance GetValuePtr(int32_t aOffset);

    [[nodiscard]] BufferStats GetStats() const;

    void Invalidate();

private:
    struct FlatTypeInfo
    {
        Red::CBaseRTTIType* type;
        uintptr_t offset;
    };

    using FlatValueMap = Core::Map<uint64_t, int32_t>; // ValueHash -> BufferOffset
    using FlatPoolMap = Core::Map<Red::CName, FlatValueMap>; // TypeName -> FlatPool
    using FlatDefaultMap = Core::Map<Red::CName, int32_t>; // TypeName -> BufferOffset
    using FlatTypeMap = Core::Map<uintptr_t, FlatTypeInfo>; // VFT -> FlatTypeInfo

    void Initialize();
    void UpdateStats(float updateTime = 0);
    void SyncBuffer();

    inline Red::CStackType GetFlatData(int32_t aOffset);
    inline static uint64_t Hash(const Red::CBaseRTTIType* aType, Red::ScriptInstance aValue);

    Red::TweakDB* m_tweakDb;
    uintptr_t m_bufferEnd;
    uintptr_t m_offsetEnd;
    FlatPoolMap m_pools;
    FlatDefaultMap m_defaults;
    FlatTypeMap m_vfts;
    BufferStats m_stats;
};
}
