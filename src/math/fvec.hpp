#ifndef MATH_VEC3_HPP
#define MATH_VEC3_HPP

#include "math/genvec.hpp"

namespace math {

using vec3 = genvec<real, 3>;

using point3_t = vec3;

using direction3_t = vec3; // a unit vector

using normal3_t = vec3; // a unit vector, perdendicular to some surface

static_assert(std::is_trivially_default_constructible_v<vec3>);
static_assert(std::is_trivially_copyable_v<vec3>);
static_assert(std::is_trivially_destructible_v<vec3>);

using vec3_t = vec3;

} // namespace math

#endif
