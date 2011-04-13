#ifndef MATH_QUATERNION_HPP
#define MATH_QUATERNION_HPP

#include "math/vec4/type.hpp"

namespace math {

struct quat_t {
    union {
        struct {
            float a, b, c, d;
        };

        vec4_t coeff;
    };
};

typedef quat_t quat1_t; // a unit quaternion

} // namespace math

#endif
