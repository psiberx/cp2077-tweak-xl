#pragma once

#include <cstdint>
#include <string>
#include <type_traits>

namespace Util::Str
{
template<typename T>
requires std::is_integral_v<T>
bool ParseInt(const std::string& aIn, T& aOut, int aRadix = 10)
{
    using Temp = std::conditional_t<std::is_signed_v<T>, int64_t, uint64_t>;

    Temp out;
    char* end;

    if constexpr (std::is_signed_v<T>)
        out = std::strtoll(aIn.c_str(), &end, aRadix);
    else
        out = std::strtoull(aIn.c_str(), &end, aRadix);

    if (end != aIn.c_str() + aIn.size())
        return false;

    aOut = static_cast<T>(out);
    return true;
}
};
