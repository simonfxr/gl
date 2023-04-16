#ifndef MATH_FVEC_HPP
#define MATH_FVEC_HPP

#include "math/vec.hpp"

namespace math {

using vec2 = vec<real, 2>;

inline vec2
cossin(real rad)
{
    return { cos(rad), sin(rad) };
}

using vec3 = vec<real, 3>;
using point3 = vec3;     // coordinates of a point
using direction3 = vec3; // a unit vector
using normal3 = vec3;    // a unit vector, perdendicular to some surface

using vec4 = vec<real, 4>;

static_assert(std::is_trivially_default_constructible_v<vec3>);
static_assert(std::is_trivially_copyable_v<vec3>);
static_assert(std::is_trivially_destructible_v<vec3>);

} // namespace math

#endif
