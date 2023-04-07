#pragma once

namespace Red
{
template<typename T, typename U = T>
inline WeakHandle<U>& AsWeakHandle(T* aInstance)
{
    return *reinterpret_cast<WeakHandle<U>*>(&aInstance->ref);
}

template<typename T, typename U = T>
inline Handle<U> AsHandle(T* aInstance)
{
    if (aInstance->ref.instance)
    {
        return reinterpret_cast<WeakHandle<U>*>(&aInstance->ref)->Lock();
    }
    else
    {
        return MakeHandle<U>(aInstance);
    }
}
}
