#ifndef MATH_VEC4_HPP
#define MATH_VEC4_HPP

#include "math/genvec.hpp"

namespace math {

using vec4_t = genvec<real, 4>;

inline constexpr vec4_t
vec4(real x, real y, real z, real w)
{
    return vec4_t::make(x, y, z, w);
}

inline constexpr vec4_t
vec4(const genvec<real, 3> &xyz, real w)
{
    return vec4_t::make(xyz[0], xyz[1], xyz[2], w);
}

template<typename T>
inline constexpr vec4_t
vec4(const genvec<T, 4> &v)
{
    return vec4_t::convert(v);
}

inline constexpr vec4_t
vec4(real x)
{
    return vec4_t::fill(x);
}

inline constexpr vec4_t
vec4(const vec4_t::buffer b)
{
    return vec4_t::load(b);
}

} // namespace math

#endif
