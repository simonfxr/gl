#ifndef SYS_ENDIAN_HPP
#define SYS_ENDIAN_HPP

#include "defs.h"

#include <hu/endian.h>

#include "bl/type_traits.hpp"

namespace sys {

inline constexpr uint8_t
bswap(uint8_t x)
{
    return x;
}

inline constexpr uint16_t
bswap(uint16_t x)
{
    return uint16_t((x << 8) | (x >> 8));
}

inline constexpr uint32_t
bswap(uint32_t x)
{
    x = (x >> 16) | (x << 16);
    x = ((x >> 8) & 0x00FF00FFu) | ((x << 8) & 0xFF00FF00u);
    return x;
}

inline constexpr uint64_t
bswap(uint64_t x)
{
    x = (x >> 32) | (x << 32);
    x =
      ((x >> 16) & 0x0000FFFF0000FFFFull) | ((x << 16) & 0xFFFF0000FFFF0000ull);
    x = ((x >> 8) & 0x00FF00FF00FF00FFull) | ((x << 8) & 0xFF00FF00FF00FF00ull);
    return x;
}

template<typename T, typename = bl::enable_if_t<bl::is_signed_v<T>>>
inline constexpr T
bswap(T x)
{
    using U = bl::make_unsigned_t<bl::decay_t<T>>;
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
