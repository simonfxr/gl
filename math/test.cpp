
#include "v4.hpp"
#include "vec4.hpp"
#include "mat4.hpp"

typedef float m4x4[4][4];
typedef float floatx4[4];

// namespace {
    
//     void load_identity(m4x4 m) {
//         for (uint32 i = 0; i < 4; ++i)
//             for (uint32 j = 0; j < 4; ++j)
//                 m[i][j] = i == j ? 1.f : 0.f;
//     }

//     void mul(const m4x4 M, const floatx4 a, floatx4 b) {
//         for (uint32 i = 0; i < 4; ++i) {
//             b[i] = 0.f;
//             for (uint32 j = 0; j < 4; ++j)
//                 b[i] += M[i][j] * a[j];
//         }
//     }
// }

void transform(const mat4& A, const mat4& B, vec4& v) {
    v = A * (B * v);
}

void transp(mat4& A) {
    A = A.transpose();
}

void matbaz(mat4& D, const mat4& A, const mat4& B) {
    mat4 BT = B.transpose();
    D = A * BT;
}

void baz(vec4 *r) {
    *r = (identity * vec4(1, 2, 3, 0));
}

// void baz2(floatx4 r) {
//     m4x4 I;
//     floatx4 a = { 1.f, 2.f, 3.f, 0.f };
//     load_identity(I);
//     mul(I, a, r);
// }

float blah(const vec4 a, const vec4 b) {
    return v4::dot(&a.packed, &b.packed);
}
