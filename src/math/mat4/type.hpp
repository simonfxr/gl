#ifndef MAT4_TYPE_HPP
#define MAT4_TYPE_HPP

#include "math/real/type.hpp"
#include "math/vec4/type.hpp"

namespace math {

struct mat4_t {

    union {
        vec4_t columns[4];
        real components[16];
    };

MATH_FUNC     const vec4_t& operator[](defs::index) const PURE_FUNC;
MATH_FUNC     vec4_t& operator[](defs::index) MUT_FUNC;
MATH_FUNC     real operator()(defs::index, defs::index) const PURE_FUNC;
MATH_FUNC     real& operator()(defs::index, defs::index) MUT_FUNC;
};

typedef mat4_t aligned_mat4_t ATTRS(ATTR_ALIGNED(16));

} // namespace math

#endif

