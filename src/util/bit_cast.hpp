#ifndef UTIL_BIT_CAST_HPP
#define UTIL_BIT_CAST_HPP 1

#include "defs.h"

#include <array>
#include <cstring>

#if hu_has_include(<bit>)
#    include <bit>
#endif

#ifdef __cpp_lib_bit_cast
#    define BIT_CAST_CONSTEXPR_P 1
#    define BIT_CAST_constexpr constexpr

template<class To, class From>
constexpr To
bit_cast(const From &src) noexcept
{
    return std::bit_cast<To>(src);
}

#else

#    include <cstring>
#    include <type_traits>
#    define BIT_CAST_CONSTEXPR_P 0
#    define BIT_CAST_constexpr

template<class To, class From>
typename std::enable_if<(sizeof(To) == sizeof(From)) &&
                          std::is_trivially_copyable<From>::value &&
                          std::is_trivial<To>::value,
                        To>::type
bit_cast(const From &src) noexcept
{
    To dst;
    std::memcpy(&dst, &src, sizeof(To));
    return dst;
}

#endif

template<typename From>
BIT_CAST_constexpr auto
to_bytes(const From &src) noexcept
{
    return bit_cast<std::array<char, sizeof(From)>>(src);
}

template<typename To>
To
read_bytes_as(const char *from) noexcept
{
    std::array<char, sizeof(To)> bytes;
    std::memcpy(bytes.data(), from, sizeof(bytes));
    return bit_cast<To>(bytes);
}

#endif
