#ifndef MAT4_H
#define MAT4_H

#include "vec4.hpp"

namespace math {

struct mat4 {

    union {
        struct {
            vec4_t c[4];
        };
        struct {
            vec4_t c1, c2, c3, c4;
        };
        float a[4][4];
        float flat[16];
    };

    mat4() {}

    mat4(const vec4_t& _c1, const vec4_t& _c2, const vec4_t& _c3, const vec4_t& _c4) :
        c1(_c1), c2(_c2), c3(_c3), c4(_c4)
        {}

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
                C.a[i][j] = dot(A.c[i], B.c[j]);

        return C;
    }

    mat4 operator *(const mat4& B) const {
        return mult(*this, B);
    }

    vec4_t operator *(const vec4_t& v) const {
        mat4 AT = transpose();
        return vec4(dot(v, AT.c1), dot(v, AT.c2),
                    dot(v, AT.c3), dot(v, AT.c4));
    }
};

namespace {

const mat4 identity(vec4(1.f, 0.f, 0.f, 0.f),
                    vec4(0.f, 1.f, 0.f, 0.f), 
                    vec4(0.f, 0.f, 1.f, 0.f),
                    vec4(0.f, 0.f, 0.f, 1.f));

} // namespace anon

} // namespace math

#endif           
                 
