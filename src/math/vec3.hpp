#ifndef MATH_VEC3_HPP
#define MATH_VEC3_HPP

#include "math/genvec.hpp"

namespace math {

using vec3_t = genvec<real, 3>;

inline constexpr vec3_t
vec3(real x, real y, real z)
{
    return vec3_t::make(x, y, z);
}

template<typename T>
inline constexpr vec3_t
vec3(const genvec<T, 3> &v)
{
    return vec3_t::convert(v);
}

inline constexpr vec3_t
vec3(real x)
{
    return vec3_t::fill(x);
}

inline constexpr vec3_t
vec3(const vec3_t::buffer b)
{
    return vec3_t::load(b);
}

template<typename T>
inline constexpr vec3_t
vec3(const genvec<T, 4> &v)
{
    return vec3(v[0], v[1], v[2]);
}

typedef vec3_t point3_t;

typedef vec3_t direction3_t; // a unit vector

typedef vec3_t normal3_t; // a unit vector, perdendicular to some surface
} // namespace math

#endif
