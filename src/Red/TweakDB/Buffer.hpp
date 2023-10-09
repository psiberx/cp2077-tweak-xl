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

    int32_t AllocateValue(const Red::Value<>& aData);
    int32_t AllocateValue(const Red::CBaseRTTIType* aType, Red::Instance aInstance);
    int32_t AllocateDefault(const Red::CBaseRTTIType* aType);

    Red::Value<> GetValue(int32_t aOffset);
    Red::Instance GetValuePtr(int32_t aOffset);
    uint64_t GetValueHash(int32_t aOffset);

    [[nodiscard]] BufferStats GetStats() const;

    void Invalidate();

    inline static uint64_t ComputeHash(const Red::CBaseRTTIType* aType, Red::Instance aInstance,
                                       uint64_t aSeed = 0xCBF29CE484222325);

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

    inline Red::Value<> ResolveOffset(int32_t aOffset);

    void CreatePools();
    void FillDefaults();
    void SyncBufferData();
    void SyncBufferBounds();
    void UpdateStats(float updateTime = 0);

    Red::TweakDB* m_tweakDb;
    FlatPoolMap m_pools;
    FlatDefaultMap m_defaults;
    FlatTypeMap m_types;
    uintptr_t m_bufferEnd;
    uintptr_t m_offsetEnd;
    BufferStats m_stats;
    std::shared_mutex m_poolMutex;
};
}
