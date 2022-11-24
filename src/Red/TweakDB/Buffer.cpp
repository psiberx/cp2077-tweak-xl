#include "Buffer.hpp"
#include "Reflection.hpp"

namespace
{
constexpr auto FlatVFTSize = 8u;
constexpr auto FlatAlignment = 8u;

const std::array<Red::CName, 26> s_flatTypes = {
    Red::ERTDBFlatType::Int,
    Red::ERTDBFlatType::Float,
    Red::ERTDBFlatType::Bool,
    Red::ERTDBFlatType::String,
    Red::ERTDBFlatType::CName,
    Red::ERTDBFlatType::TweakDBID,
    Red::ERTDBFlatType::LocKey,
    Red::ERTDBFlatType::ResRef,
    Red::ERTDBFlatType::Quaternion,
    Red::ERTDBFlatType::EulerAngles,
    Red::ERTDBFlatType::Vector3,
    Red::ERTDBFlatType::Vector2,
    Red::ERTDBFlatType::Color,
    Red::ERTDBFlatType::IntArray,
    Red::ERTDBFlatType::FloatArray,
    Red::ERTDBFlatType::BoolArray,
    Red::ERTDBFlatType::StringArray,
    Red::ERTDBFlatType::CNameArray,
    Red::ERTDBFlatType::TweakDBIDArray,
    Red::ERTDBFlatType::LocKeyArray,
    Red::ERTDBFlatType::ResRefArray,
    Red::ERTDBFlatType::QuaternionArray,
    Red::ERTDBFlatType::EulerAnglesArray,
    Red::ERTDBFlatType::Vector3Array,
    Red::ERTDBFlatType::Vector2Array,
    Red::ERTDBFlatType::ColorArray,
};
}

Red::TweakDBBuffer::TweakDBBuffer()
    : TweakDBBuffer(Red::TweakDB::Get())
{
}

Red::TweakDBBuffer::TweakDBBuffer(Red::TweakDB* aTweakDb)
    : m_tweakDb(aTweakDb)
    , m_bufferEnd(0)
    , m_offsetEnd(0)
{
}

int32_t Red::TweakDBBuffer::AllocateValue(const Red::Value<>& aData)
{
    return AllocateValue(aData.type, aData.instance);
}

int32_t Red::TweakDBBuffer::AllocateValue(const Red::CBaseRTTIType* aType, Red::Instance aInstance)
{
    if (m_bufferEnd != m_tweakDb->flatDataBufferEnd)
        SyncBufferData();

    auto& pool = m_pools.at(aType->GetName());
    const auto hash = ComputeHash(aType, aInstance);

    {
        std::shared_lock poolLockR(m_poolMutex);
        const auto offsetIt = pool.find(hash);
        if (offsetIt != pool.end())
            return offsetIt->second;
    }

    const auto offset = m_tweakDb->CreateFlatValue({ const_cast<Red::CBaseRTTIType*>(aType), aInstance});

    if (offset > 0)
    {
        std::unique_lock poolLockRW(m_poolMutex);
        pool.emplace(hash, offset);
    }

    return offset;
}

int32_t Red::TweakDBBuffer::AllocateDefault(const Red::CBaseRTTIType* aType)
{
    if (m_bufferEnd != m_tweakDb->flatDataBufferEnd)
        SyncBufferData();

    return m_defaults.at(aType->GetName());
}

Red::Value<> Red::TweakDBBuffer::GetValue(int32_t aOffset)
{
    if (aOffset < 0)
        return {};

    if (m_bufferEnd != m_tweakDb->flatDataBufferEnd)
        SyncBufferData();

    return ResolveOffset(aOffset);
}

Red::Instance Red::TweakDBBuffer::GetValuePtr(int32_t aOffset)
{
    if (aOffset < 0)
        return {};

    if (m_bufferEnd != m_tweakDb->flatDataBufferEnd)
        SyncBufferData();

    return ResolveOffset(aOffset).instance;
}

uint64_t Red::TweakDBBuffer::GetValueHash(int32_t aOffset)
{
    if (aOffset < 0)
        return {};

    if (m_bufferEnd != m_tweakDb->flatDataBufferEnd)
        SyncBufferData();

    const auto data = ResolveOffset(aOffset);

    return ComputeHash(data.type, data.instance);
}

uint64_t Red::TweakDBBuffer::ComputeHash(const Red::CBaseRTTIType* aType, Red::Instance aInstance)
{
    // Case 1: Everything is processed as a sequence of bytes and passed to the hash function,
    //         except for an array of strings.
    // Case 2: Arrays of strings are different because of empty strings that don't produce any
    //         hashing value. Therefore hash will be equal for different arrays in cases like:
    //         [] == [""] == ["", ""]
    //         ["", "a", "b"] == ["a", "", "b"] == ["a", "b", ""]
    //         As a workaround, we hash the string length as part of the data.

    uint64_t hash;

    if (aType->GetType() == Red::ERTTIType::Array)
    {
        auto* arrayType = reinterpret_cast<const Red::CRTTIArrayType*>(aType);
        auto* innerType = arrayType->GetInnerType();

        if (innerType->GetName() == "String")
        {
            const auto* array = reinterpret_cast<Red::DynArray<Red::CString>*>(aInstance);
            hash = Red::FNV1a64(reinterpret_cast<uint8_t*>(0), 0); // Initial seed
            for (uint32_t i = 0; i != array->size; ++i)
            {
                const auto* str = array->entries + i;
                const auto length = str->Length();
                hash = Red::FNV1a64(reinterpret_cast<const uint8_t*>(&length), sizeof(length), hash);
                hash = Red::FNV1a64(reinterpret_cast<const uint8_t*>(str->c_str()), length, hash);
            }
        }
        else
        {
            const auto* array = reinterpret_cast<Red::DynArray<uint8_t>*>(aInstance);
            hash = Red::FNV1a64(array->entries, array->size * innerType->GetSize());
        }
    }
    else if (aType->GetName() == "String")
    {
        const auto* str = reinterpret_cast<Red::CString*>(aInstance);
        const auto* data = reinterpret_cast<const uint8_t*>(str->c_str());
        hash = Red::FNV1a64(data, str->Length());
    }
    else
    {
        const auto* data = reinterpret_cast<const uint8_t*>(aInstance);
        hash = Red::FNV1a64(data, aType->GetSize());
    }

    return hash;
}

Red::Value<> Red::TweakDBBuffer::ResolveOffset(int32_t aOffset)
{
    // This method uses VFTs to determine the flat type.
    // It's 11% to 33% faster than calling GetValue() every time.

    const auto addr = m_tweakDb->flatDataBuffer + aOffset;
    const auto vft = *reinterpret_cast<uintptr_t*>(addr);
    const auto it = m_types.find(vft);

    // For a known VFT we can immediately get RTTI type and data pointer.
    if (it != m_types.end())
        return { it->second.type, reinterpret_cast<void*>(addr + it->second.offset) };

    // For an unknown VFT, we call the virtual GetValue() once to get the type.
    const auto data = reinterpret_cast<TweakDBFlatValue*>(addr)->GetValue();

    // Add type info to the map.
    // In addition to the RTTI type, we also store the data offset considering alignment.
    // Quaternion is 16-byte aligned, so there is 8-byte padding between the VFT and the data:
    // [ 8B VFT ][ 8B PAD ][ 16B QUATERNION ]
    m_types.insert({ vft, { data.type, std::max(data.type->GetAlignment(), FlatAlignment) } });

    return data;
}

void Red::TweakDBBuffer::CreatePools()
{
    if (m_pools.size() != s_flatTypes.size())
    {
        m_pools.reserve(s_flatTypes.size());

        for (const auto& typeName : s_flatTypes)
        {
            m_pools.emplace(typeName, 0);
        }
    }
}

void Red::TweakDBBuffer::FillDefaults()
{
    if (m_defaults.size() != s_flatTypes.size())
    {
        for (const auto& typeName : s_flatTypes)
        {
            auto& pool = m_pools.at(typeName);

            const auto value = Red::MakeValue(typeName);
            const auto hash = ComputeHash(value->type, value->instance);
            const auto it = pool.find(hash);

            int32_t offset = -1;

            if (it != pool.end())
            {
                offset = it->second;
            }
            else
            {
                offset = m_tweakDb->CreateFlatValue(*value);
                pool.emplace(hash, offset);
            }

            m_defaults.emplace(typeName, offset);

            ResolveOffset(static_cast<int32_t>(offset));
        }
    }
}

void Red::TweakDBBuffer::SyncBufferData()
{
    std::unique_lock poolLockRW(m_poolMutex);

    auto offsetEnd = m_tweakDb->flatDataBufferEnd - m_tweakDb->flatDataBuffer;

    if (m_offsetEnd == offsetEnd)
    {
        SyncBufferBounds();
        return;
    }

    if (m_offsetEnd == 0)
        CreatePools();

    const auto startTimePoint = std::chrono::steady_clock::now();

    {
        std::shared_lock flatLockR(m_tweakDb->mutex00);

        auto offset = Red::AlignUp(static_cast<uint32_t>(m_offsetEnd), FlatAlignment);
        while (offset < offsetEnd)
        {
            // The current offset should always point to the VFT of the next flat.
            // If there's zero instead, that means the next value is 16-byte aligned,
            // and we need to skip the 8-byte padding to get to the flat.
            if (*reinterpret_cast<uint64_t*>(m_tweakDb->flatDataBuffer + offset) == 0ull)
                 offset += 8u;

            const auto data = ResolveOffset(static_cast<int32_t>(offset));
            const auto hash = ComputeHash(data.type, data.instance);

            auto& pool = m_pools.at(data.type->GetName());

            // Check for duplicates...
            // (Original game's blob has ~24K duplicates)
            if (!pool.contains(hash))
                pool.emplace(hash, offset);

            // Step {vft + data_size} aligned by {max(data_align, 8)}
            offset += Red::AlignUp(FlatVFTSize + data.type->GetSize(),
                                   std::max(FlatAlignment, data.type->GetAlignment()));
        }
    }

    const auto endTimePoint = std::chrono::steady_clock::now();
    const auto updateTime = std::chrono::duration_cast<std::chrono::duration<float>>(endTimePoint - startTimePoint).count();

    if (m_offsetEnd == 0)
        FillDefaults();

    SyncBufferBounds();

    UpdateStats(updateTime);
}

void Red::TweakDBBuffer::SyncBufferBounds()
{
    m_bufferEnd = m_tweakDb->flatDataBufferEnd;
    m_offsetEnd = m_tweakDb->flatDataBufferEnd - m_tweakDb->flatDataBuffer;
}

void Red::TweakDBBuffer::UpdateStats(float updateTime)
{
    if (updateTime != 0)
    {
        if (m_stats.initTime == 0)
            m_stats.initTime = updateTime;
        else
            m_stats.updateTime = updateTime;
    }

    size_t totalValues = 0;
    for (const auto& pool : m_pools)
        totalValues += pool.second.size();

    m_stats.poolSize = m_offsetEnd;
    m_stats.poolValues = totalValues;
    m_stats.knownTypes = m_types.size();
    m_stats.flatEntries = m_tweakDb->flats.size;

#ifdef VERBOSE
    Red::Log::Debug(
        "[Red::TweakDBFlatPool] init {:.3f}s | update {:.6f}s | {} KiB | {} values | {} flats | {} types",
        m_stats.initTime, m_stats.updateTime,
        m_stats.poolSize / 1024, m_stats.poolValues,
        m_stats.flatEntries, m_stats.knownTypes);
#endif
}

Red::TweakDBBuffer::BufferStats Red::TweakDBBuffer::GetStats() const
{
    return m_stats;
}

void Red::TweakDBBuffer::Invalidate()
{
    std::unique_lock poolLockRW(m_poolMutex);

    m_bufferEnd = 0;
    m_offsetEnd = 0;

    for (const auto& typeName : s_flatTypes)
    {
        m_pools.at(typeName).clear();
    }

    m_stats = {};
}
