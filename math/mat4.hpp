#ifndef MAT4_H
#define MAT4_H

#include <xmmintrin.h>
#include <smmintrin.h>

#include "Math.hpp"
#include "vec4.hpp"

struct mat4 {

    union {
        struct {
            __m128 c[4];
        };
        struct {
            __m128 c1, c2, c3, c4;
        };
        float a[4][4];
        float flat[16];
    };

    mat4() {}

    mat4(const vec4& _c1, const vec4& _c2, const vec4& _c3, const vec4& _c4)
        : c1(_c1.packed), c2(_c2.packed), c3(_c3.packed), c4(_c4.packed) {}

    mat4(const mat4& m)
        : c1(m.c1), c2(m.c2), c3(m.c3), c4(m.c4) {}

    static float dot4(const vec4& a, const vec4& b) {
        // return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
        return _mm_cvtss_f32(_mm_dp_ps(a.packed, b.packed, 0xF1));
    }

    mat4 transpose() const {
        mat4 r;
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                r.a[i][j] = a[j][i];
        return r;
    }

    static mat4 mult(const mat4& A, const mat4& B) {
        mat4 BT = B.transpose();
        mat4 C;

        for (uint32 i = 0; i < 4; ++i)
            for (uint32 j = 0; j < 4; ++j)
                C.a[i][j] = dot4(A.c[i], B.c[j]);

        return C;
    }

    mat4 operator *(const mat4& B) const {
        return mult(*this, B);
    }

    vec4 operator *(const vec4& v) const {
        mat4 AT = transpose();
        return vec4(dot4(v, AT.c1), dot4(v, AT.c2), dot4(v, AT.c3), dot4(v, AT.c4));
    }
};

namespace {
    const mat4 identity(vec4(1.f, 0.f, 0.f, 0.f),
                        vec4(0.f, 1.f, 0.f, 0.f), 
                        vec4(0.f, 0.f, 1.f, 0.f),
                        vec4(0.f, 0.f, 0.f, 1.f));

}

#endif           
                 
