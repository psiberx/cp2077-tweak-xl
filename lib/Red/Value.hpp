#pragma once

namespace Red
{
template<typename T = void>
using ValuePtr = std::shared_ptr<T>;

template<typename T, typename... Args>
auto MakeValue(Args&&... args)
{
    return std::allocate_shared<T>(TiltedPhoques::StlAllocator<T>(), std::forward<Args>(args)...);
}
}
