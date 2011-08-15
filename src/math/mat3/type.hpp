#ifndef MAT3_TYPE_HPP
#define MAT3_TYPE_HPP

#include "math/real/type.hpp"
#include "math/vec3/type.hpp"

namespace math {

struct mat3_t {

    union {
        vec3_t columns[3];
        real components[9];
    };

    const vec3_t& operator[](defs::index) const PURE_FUNC;
    vec3_t& operator[](defs::index) MUT_FUNC;
    float& operator()(defs::index, defs::index) MUT_FUNC;
};

typedef mat3_t aligned_mat3_t ATTRS(ATTR_ALIGNED(16));

} // namespace math

#endif

