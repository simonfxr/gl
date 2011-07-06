#ifndef MAT4_TYPE_HPP
#define MAT4_TYPE_HPP

#include "defs.h"
#include "math/vec4/type.hpp"

namespace math {

struct mat4_t {

    union {
        vec4_t columns[4];
        float components[16];
    };

    const vec4_t& operator[](unsigned long i) const PURE_FUNC;
    vec4_t& operator[](unsigned long i) MUT_FUNC;
    float operator()(unsigned long i, unsigned long j) const PURE_FUNC;
    float& operator()(unsigned long i, unsigned long j) MUT_FUNC;
};

typedef mat4_t aligned_mat4_t ATTRS(ATTR_ALIGNED(16));

} // namespace math

#endif

