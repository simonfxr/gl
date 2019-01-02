#ifndef SYS_ENDIAN_HPP
#define SYS_ENDIAN_HPP

#include "defs.hpp"

#include <type_traits>

namespace sys {

inline constexpr defs::uint8_t
bswap(defs::uint8_t x)
{
    return x;
}

inline constexpr defs::uint16_t
bswap(defs::uint16_t x)
{
    return (x << 8) | (x >> 8);
}

inline constexpr defs::uint32_t
bswap(defs::uint32_t x)
{
    x = (x >> 16) | (x << 16);
    x = ((x >> 8) & 0x00FF00FFu) | ((x << 8) & 0xFF00FF00u);
    return x;
}

inline constexpr defs::uint64_t
bswap(defs::uint64_t x)
{
    x = (x >> 32) | (x << 32);
    x =
      ((x >> 16) & 0x0000FFFF0000FFFFull) | ((x << 16) & 0xFFFF0000FFFF0000ull);
    x = ((x >> 8) & 0x00FF00FF00FF00FFull) | ((x << 8) & 0xFF00FF00FF00FF00ull);
    return x;
}

template<typename T, typename = std::enable_if_t<std::is_signed_v<T>>>
inline constexpr T
bswap(T x)
{
    using U = std::make_unsigned_t<std::decay_t<T>>;
    return T(bswap(U(x)));
}

template<typename T>
inline constexpr T
hton(T x)
{
#if HU_LITTLE_ENDIAN_P
    return bswap(x);
#else
    return x;
#endif
}

template<typename T>
inline constexpr T
ntoh(T x)
{
#if HU_LITTLE_ENDIAN_P
    return bswap(x);
#else
    return x;
#endif
}

} // namespace sys

#endif
