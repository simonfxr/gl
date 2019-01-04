#ifndef MATH_IVEC3_HPP
#define MATH_IVEC3_HPP

#include "math/genvec.hpp"

namespace math {

using ivec3_t = genvec<int32_t, 3>;

inline constexpr ivec3_t
ivec3(int32_t x, int32_t y, int32_t z)
{
    return ivec3_t::make(x, y, z);
}

template<typename T>
inline constexpr ivec3_t
ivec3(const genvec<T, 3> &v)
{
    return ivec3_t::convert(v);
}

inline constexpr ivec3_t
ivec3(int32_t x)
{
    return ivec3_t::fill(x);
}

inline constexpr ivec3_t
ivec3(const ivec3_t::buffer b)
{
    return ivec3_t::load(b);
}

template<typename T>
inline constexpr ivec3_t
ivec3(const genvec<T, 4> &v)
{
    return ivec3(v[0], v[1], v[2]);
}
} // namespace math

#endif
