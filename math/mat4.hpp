#ifndef MAT4_H
#define MAT4_H

#include <xmmintrin.h>

#include "Math.hpp"
#include "vec4.hpp"

struct mat4;

namespace __mat4 {
    namespace {
        extern const mat4 identity;
    }
}

struct mat4 {

    vec4 c1, c2, c3, c4; // column major, like OpenGL

    mat4() {}

    mat4(const vec4& _c1, const vec4& _c2, const vec4& _c3, const vec4& _c4)
        : c1(_c1), c2(_c2), c3(_c3), c4(_c4) {}

    mat4(const mat4& m)
        : c1(m.c1), c2(m.c2), c3(m.c3), c4(m.c4) {}

    static float dot4(const vec4& a, const vec4& b) {
        vec4 s = vec4(_mm_mul_ps(a.packed, b.packed));
        return s.x + s.y + s.z + s.w;
    }

    static const mat4& identity() {
        return __mat4::identity;
    }

    mat4 transpose() const {
        return mat4(vec4(c1.x, c2.x, c3.x, c4.x),
                    vec4(c1.y, c2.y, c3.y, c4.y),
                    vec4(c1.z, c2.z, c3.z, c4.z),
                    vec4(c1.w, c2.w, c3.w, c4.w));
    }

    static mat4 mult(const mat4& A, const mat4& B) {
        mat4 BT = B.transpose();
        return mat4(vec4(dot4(A.c1, BT.c1), dot4(A.c1, BT.c2), dot4(A.c1, BT.c3), dot4(A.c1, BT.c4)),
                    vec4(dot4(A.c2, BT.c1), dot4(A.c2, BT.c2), dot4(A.c2, BT.c3), dot4(A.c2, BT.c4)),
                    vec4(dot4(A.c3, BT.c1), dot4(A.c3, BT.c2), dot4(A.c3, BT.c3), dot4(A.c3, BT.c4)),
                    vec4(dot4(A.c4, BT.c1), dot4(A.c4, BT.c2), dot4(A.c4, BT.c3), dot4(A.c4, BT.c4)));
    }

    mat4 operator *(const mat4& B) const {
        return mult(*this, B);
    }

    vec4 operator *(const vec4& v) const {
        mat4 AT = transpose();
        return vec4(dot4(v, AT.c1), dot4(v, AT.c2), dot4(v, AT.c3), dot4(v, AT.c4));
    }
};

namespace __mat4 {
    namespace {
        // should be in its own file to prevent copies...        
        const mat4 identity =
            mat4(vec4(1.f, 0.f, 0.f, 0.f),
                 vec4(0.f, 1.f, 0.f, 0.f), 
                 vec4(0.f, 0.f, 1.f, 0.f),
                 vec4(0.f, 0.f, 0.f, 1.f));
        }
}
#endif           
                 
