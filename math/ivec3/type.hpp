#ifndef MATH_IVEC3_TYPE
#define MATH_IVEC3_TYPE

#include "math/defs.hpp"

namespace math {

struct ivec3_t {
    
    union {
        struct {
            int32 i, j, k;
        };
        
        struct {
            int32 x, y, z;
        };

        struct {
            int32 r, g, b;
        };

        struct {
            int32 s, t, p;
        };
        
        int32 components[3];
    };

    int32& operator[](unsigned long i) MUT_FUNC;
    int32 operator[](unsigned long i) const PURE_FUNC;
};

} // namespace math

#endif
