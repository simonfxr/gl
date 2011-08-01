#ifndef MATH_VEC2_TYPE
#define MATH_VEC2_TYPE

#include "math/real/type.hpp"

namespace math {

struct vec2_t {
    real components[2];

    real& operator[](unsigned long i) MUT_FUNC;
    real operator[](unsigned long i) const PURE_FUNC;
};

typedef vec2_t point2_t;

typedef vec2_t direction2_t; // a unit vector

typedef vec2_t normal2_t; // a unit vector, perdendicular to a plane

} // namespace math

#endif
