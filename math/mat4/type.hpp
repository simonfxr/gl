#ifndef MAT4_TYPE_HPP
#define MAT4_TYPE_HPP

#include "math/vec4/type.hpp"

namespace math {

struct mat4_t {

    union {
        vec4_t columns[4];
        float components[16];
    };

    vec4_t operator[](unsigned long i) const PURE_FUNC;
    vec4_t& operator[](unsigned long i) MUT_FUNC;
    float& operator()(unsigned long i, unsigned long j) MUT_FUNC;
};

} // namespace math

#endif

