#ifndef MAT3_TYPE_HPP
#define MAT3_TYPE_HPP

#include "defs.h"
#include "math/vec3/type.hpp"

namespace math {

struct mat3_t {

    union {
        vec3_t columns[3];
        float components[9];
    };

    vec3_t operator[](unsigned long i) const PURE_FUNC;
    vec3_t& operator[](unsigned long i) MUT_FUNC;
    float& operator()(unsigned long i, unsigned long j) MUT_FUNC;
};

typedef mat3_t aligned_mat3_t ATTRS(ATTR_ALIGNED(16));

} // namespace math

#endif

