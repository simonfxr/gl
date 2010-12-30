#ifndef MAT3_HPP
#define MAT3_HPP

#include "defs.h"
#include "math/vec3.hpp"

struct mat3 {

    float flat[9]; // column major

    mat3() {}

    mat3(vec3 c1, vec3 c2, vec3 c3) {
        flat[0] = c1.x; flat[1] = c1.y; flat[2] = c1.z;
        flat[3] = c2.x; flat[4] = c2.y; flat[5] = c2.z;
        flat[6] = c3.x; flat[7] = c3.y; flat[8] = c3.z;
    }

    mat3(const mat3& m) {
        for (uint32 i = 0; i < 9; ++i)
            flat[i] = m.flat[i];
    }

    mat3& operator =(const mat3& m) {
        for (uint32 i = 0; i < 9; ++i)
            flat[i] = m.flat[i];
        return *this;
    }    
};

#endif
