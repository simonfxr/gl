#ifndef MATH_VEC2_HPP
#define MATH_VEC2_HPP

#include "math/genvec.hpp"

namespace math {

using vec2_t = genvec<real, 2>;

inline constexpr vec2_t
vec2(real x, real y)
{
    return vec2_t::make(x, y);
}

template<typename T>
inline constexpr vec2_t
vec2(const genvec<T, 2> &v)
{
    return vec2_t::convert(v);
}

inline constexpr vec2_t
vec2(real x)
{
    return vec2_t::fill(x);
}

inline constexpr vec2_t
vec2(const vec2_t::buffer b)
{
    return vec2_t::load(b);
}

} // namespace math

#endif
