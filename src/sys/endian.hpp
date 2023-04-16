#ifndef SYS_ENDIAN_HPP
#define SYS_ENDIAN_HPP

#include "defs.h"

#include <bl/core.hpp>

#include <bo/cxx.hpp>
#include <hu/endian.h>
#include <type_traits>

namespace sys {

inline constexpr decltype(auto)
bswap(auto &&x)
{
    return bo::bswap(bl_fwd(x));
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
