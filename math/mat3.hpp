#ifndef MAT3_HPP
#define MAT3_HPP

#include "defs.h"
#include "math/vec3.hpp"

namespace math {

struct mat3 {

    union { // column major
        
        float flat[9];
        
        struct {
            vec3_t c1, c2, c3;
        };
    };

    mat3() {}

    mat3(vec3_t _c1, vec3_t _c2, vec3_t _c3) :
        c1(_c1), c2(_c2), c3(_c3) {}
};

} // namespace math

#endif
