#pragma once

namespace Red
{
template<typename T = void>
struct ValueInstance
{
    ValueInstance(CBaseRTTIType* aType)
    {
        type = aType;
        value = type->GetAllocator()->AllocAligned(type->GetSize(), type->GetAlignment()).memory;
        std::memset(value, 0, type->GetSize());
        type->Construct(value);
    }

    ~ValueInstance()
    {
        type->Destruct(value);
        type->GetAllocator()->Free(value);
    }

    [[nodiscard]] inline T* operator->() const
    {
        return value;
    }

    [[nodiscard]] inline operator T*() const
    {
        return value;
    }

    [[nodiscard]] inline operator const CStackType&() const
    {
        return *reinterpret_cast<const CStackType*>(this);
    }

    explicit operator bool() const noexcept
    {
        return value != nullptr;
    }

    CBaseRTTIType* type;
    T* value;
};

template<typename T = void>
using ValuePtr = std::shared_ptr<const ValueInstance<T>>;

template<typename T = void>
ValuePtr<T> MakeValue(CBaseRTTIType* aType)
{
    return std::make_shared<const ValueInstance<T>>(aType);
}

template<typename T = void>
ValuePtr<T> MakeValue(CName aTypeName)
{
    return MakeValue<T>(CRTTISystem::Get()->GetType(aTypeName));
}
}
