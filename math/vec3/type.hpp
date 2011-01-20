#ifndef VEC3_TYPE
#define VEC3_TYPE

#include "math/defs.hpp"

namespace math {

struct vec3_t {
    
    union {
        struct {
            float x, y, z;
        };
        float components[3];
    };

#ifndef MATH_INLINE
    float& MUT_FUNC operator[](unsigned long i);
    float  PURE_FUNC operator[](unsigned long i) const;
#else
    
    float& MUT_FUNC operator[](unsigned long i) {
        return components[i];
    }

    float PURE_FUNC operator[](unsigned long i) const {
        return components[i];
    }

#endif
};

} // namespace math

#endif
