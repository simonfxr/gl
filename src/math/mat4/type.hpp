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

    const vec4_t& operator[](index_t) const PURE_FUNC;
    vec4_t& operator[](index_t) MUT_FUNC;
    real operator()(index_t, index_t) const PURE_FUNC;
    real& operator()(index_t, index_t) MUT_FUNC;
};

typedef mat4_t aligned_mat4_t ATTRS(ATTR_ALIGNED(16));

} // namespace math

#endif

