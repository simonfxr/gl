#ifndef VEC4_TYPE
#define VEC4_TYPE

#include "math/real/type.hpp"

namespace math {

struct vec4_t {
    real components[4];

    real& operator[](defs::index) MUT_FUNC;
    real operator[](defs::index) const PURE_FUNC;
};

typedef vec4_t point4_t;

typedef vec4_t ATTRS(ATTR_ALIGNED(16)) aligned_vec4_t;

typedef aligned_vec4_t aligned_point4_t;

} // namespace math

#endif
