#ifndef MATH_VEC2_TYPE
#define MATH_VEC2_TYPE

#include "math/real/type.hpp"

namespace math {

struct vec2_t {
    real components[2];

    real& operator[](defs::index) MUT_FUNC;
    real operator[](defs::index) const PURE_FUNC;
};

typedef vec2_t point2_t;

typedef vec2_t direction2_t;

typedef vec2_t normal2_t;

} // namespace math

#endif
