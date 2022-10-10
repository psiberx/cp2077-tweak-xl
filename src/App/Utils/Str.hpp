#pragma once

namespace App
{
template<typename T>
requires std::is_integral_v<T>
bool ParseInt(const std::string& aIn, T& aOut, const int aRadix = 10)
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

template<typename T>
requires std::is_integral_v<T>
T ParseInt(const std::string& aIn, const int aRadix = 10)
{
    using Temp = std::conditional_t<std::is_signed_v<T>, int64_t, uint64_t>;

    Temp out;

    if constexpr (std::is_signed_v<T>)
        out = std::strtoll(aIn.c_str(), nullptr, aRadix);
    else
        out = std::strtoull(aIn.c_str(), nullptr, aRadix);

    return static_cast<T>(out);
}

inline bool IsNumeric(const std::string& aIn, size_t aStart = 0)
{
    return aIn.find_first_not_of("0123456789", aStart) == std::string::npos;
}
};
