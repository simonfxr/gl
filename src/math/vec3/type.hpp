#ifndef VEC3_TYPE
#define VEC3_TYPE

#include "math/real/type.hpp"

namespace math {

struct vec3_t {
    real components[3];

    real& operator[](defs::index) MUT_FUNC;
    real operator[](defs::index) const PURE_FUNC;
};

typedef vec3_t point3_t;

typedef vec3_t direction3_t; // a unit vector

typedef vec3_t normal3_t; // a unit vector, perdendicular to some surface

} // namespace math

#endif
