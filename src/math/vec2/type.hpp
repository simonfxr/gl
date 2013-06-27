#ifndef MATH_VEC2_TYPE
#define MATH_VEC2_TYPE

#include "math/real/type.hpp"

namespace math {

struct vec2_t {
    static const defs::size size = 2;
    static const defs::size padded_size = 2;
    typedef real component_type;
    typedef component_type buffer[size];

    MATH_FUNC real& operator[](defs::index) MUT_FUNC;
    MATH_FUNC real operator[](defs::index) const PURE_FUNC;

private:
    real components[2];
};

typedef vec2_t point2_t;

typedef vec2_t direction2_t;

typedef vec2_t normal2_t;

} // namespace math

#endif
