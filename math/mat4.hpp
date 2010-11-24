#ifndef MAT4_H
#define MAT4_H

#include "Math.hpp"
#include "v4.hpp"
#include "vec4.hpp"

struct mat4 {

    union {
        struct {
            v4::v4 c[4];
        };
        struct {
            v4::v4 c1, c2, c3, c4;
        };
        float a[4][4];
        float flat[16];
    };

    mat4() {}

    mat4(const vec4& _c1, const vec4& _c2, const vec4& _c3, const vec4& _c4)
        : c1(_c1.packed), c2(_c2.packed), c3(_c3.packed), c4(_c4.packed) {}

    mat4(const mat4& m)
        : c1(m.c1), c2(m.c2), c3(m.c3), c4(m.c4) {}

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
                C.a[i][j] = v4::dot(&A.c[i], &B.c[j]);

        return C;
    }

    mat4 operator *(const mat4& B) const {
        return mult(*this, B);
    }

    vec4 operator *(const vec4& v) const {
        mat4 AT = transpose();
        return vec4(v4::dot(&v.packed, &AT.c1), v4::dot(&v.packed, &AT.c2),
                    v4::dot(&v.packed, &AT.c3), v4::dot(&v.packed, &AT.c4));
    }
};

namespace {
    const mat4 identity(vec4(1.f, 0.f, 0.f, 0.f),
                        vec4(0.f, 1.f, 0.f, 0.f), 
                        vec4(0.f, 0.f, 1.f, 0.f),
                        vec4(0.f, 0.f, 0.f, 1.f));

}

#endif           
                 
