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

    mat4(const vec4& _c1, const vec4& _c2, const vec4& _c3, const vec4& _c4) :
        c1(v4::make(_c1.x, _c1.y, _c1.z, _c1.w)),
        c2(v4::make(_c2.x, _c2.y, _c2.z, _c2.w)),
        c3(v4::make(_c3.x, _c3.y, _c3.z, _c3.w)),
        c4(v4::make(_c4.x, _c4.y, _c4.z, _c4.w))
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
                C.a[i][j] = v4::dot(A.c[i], B.c[j]);

        return C;
    }

    mat4 operator *(const mat4& B) const {
        return mult(*this, B);
    }

    vec4 operator *(const vec4& v) const {
        mat4 AT = transpose();
        v4::v4 packed = v4::make(v.x, v.y, v.z, v.w);
        return vec4(v4::dot(packed, AT.c1), v4::dot(packed, AT.c2),
                    v4::dot(packed, AT.c3), v4::dot(packed, AT.c4));
    }
};

namespace {
    const mat4 identity(vec4(1.f, 0.f, 0.f, 0.f),
                        vec4(0.f, 1.f, 0.f, 0.f), 
                        vec4(0.f, 0.f, 1.f, 0.f),
                        vec4(0.f, 0.f, 0.f, 1.f));

}

#endif           
                 
