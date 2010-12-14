#ifndef IVEC3_HPP

#include "defs.h"
#include "math/vec3.hpp"

struct ivec3 {

    int32 x, y, z;
    
    ivec3() {}

    explicit ivec3(int32 a)
        : x(a), y(a), z(a) {}

    ivec3(int32 _x, int32 _y, int32 _z)
        : x(_x), y(_y), z(_z) {}

    ivec3(const ivec3& v)
        : x(v.x), y(v.y), z(v.z) {}

    explicit ivec3(const vec3& v)
        : x((int32) v.x), y((int32) v.y), z((int32) v.z) {}

    static ivec3 max(const ivec3& a, const ivec3& b) {
        return ivec3(b.x > a.x ? b.x : a.x,
                     b.y > a.y ? b.y : a.y,
                     b.z > a.z ? b.z : a.z);
    }

    static ivec3 min(const ivec3& a, const ivec3& b) {
        return ivec3(b.x < a.x ? b.x : a.x,
                     b.y < a.y ? b.y : a.y,
                     b.z < a.z ? b.z : a.z);
    }

};

#endif
