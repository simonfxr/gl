#ifndef VEC3_TYPE
#define VEC3_TYPE

#include "math/real/type.hpp"

namespace math {

struct vec3_t {
    real components[3];

    real& operator[](unsigned long i) MUT_FUNC;
    real operator[](unsigned long i) const PURE_FUNC;
};

typedef vec3_t point3_t;

typedef vec3_t direction3_t; // a unit vector

typedef vec3_t normal3_t; // a unit vector, perdendicular to a plane

} // namespace math

#endif
