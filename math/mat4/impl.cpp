#if defined(MATH_MAT4_INLINE) || !defined(MATH_INLINE)

#include "defs.h"
#include "math/mat4/defns.hpp"
#include "math/vec4.hpp"
#include "math/vec3.hpp"
#include "math/math.hpp"

#include "glt/utils.hpp"

MATH_BEGIN_NAMESPACE

mat4_t mat4() {
    return mat4(vec4(1.f, 0.f, 0.f, 0.f),
                vec4(0.f, 1.f, 0.f, 0.f),
                vec4(0.f, 0.f, 1.f, 0.f),
                vec4(0.f, 0.f, 0.f, 1.f));
}

mat4_t mat4(float x) {
    return mat4(vec4(x), vec4(x), vec4(x), vec4(x));
}

mat4_t mat4(const float mat[16]) {
    mat4_t A;
    for (uint32 i = 0; i < 16; ++i)
        A.components[i] = mat[i];
    return A;
}

mat4_t mat4(const mat3_t& m) {
    return mat4(vec4(m[0], 0.f), vec4(m[1], 0.f), vec4(m[2], 0.f), vec4(0.f, 0.f, 0.f, 1.f));
}

mat4_t mat4(const vec4_t& c1, const vec4_t& c2, const vec4_t& c3, const vec4_t& c4) {
    mat4_t A; A[0] = c1; A[1] = c2; A[2] = c3; A[3] = c4; return A;
}

mat4_t operator +(const mat4_t& A, const mat4_t& B) {
    return mat4(A[0] + B[0], A[1] + B[1], A[2] + B[2], A[3] + B[3]);
}

mat4_t operator -(const mat4_t& A, const mat4_t& B) {
    return mat4(A[0] - B[0], A[1] - B[1], A[2] - B[2], A[3] - B[3]);
}

mat4_t operator *(const mat4_t& A, const mat4_t& B) {
    mat4_t AT = transpose(A);
    mat4_t C;
    for (uint32 i = 0; i < 4; ++i)
        for (uint32 j = 0; j < 4; ++j)
            C(i, j) = dot(AT[j], B[i]);
    return C;
}

vec4_t operator *(const mat4_t& A, const vec4_t& v) {
    return v[0] * A[0] + v[1] * A[1] +
           v[2] * A[2] + v[3] * A[3];
    // mat4_t AT = transpose(A);
    // return vec4(dot(v, AT[0]), dot(v, AT[1]),
    //             dot(v, AT[2]), dot(v, AT[3]));
}

mat4_t operator *(const mat4_t& A, float x) {
    return mat4(A[0] * x, A[1] * x, A[2] * x, A[3] * x);
}

mat4_t operator *(float x, const mat4_t& A) {
    return A * x;
}

mat4_t operator /(const mat4_t& A, float x) {
    return A * math::recip(x);
}

mat4_t& operator +=(mat4_t& A, const mat4_t& B) {
    return A = A + B;
}

mat4_t& operator -=(mat4_t& A, const mat4_t& B) {
    return A = A - B;
}

mat4_t& operator *=(mat4_t& A, float x) {
    return A = A * x;
}

mat4_t& operator *=(mat4_t& A, const mat4_t& B) {
    return A = A * B;
}

mat4_t& operator /=(mat4_t& A, float x) {
    return A = A / x;
}

vec4_t transform(const mat4_t& A, const vec4_t& v) {
    return A * v;
}

vec3_t transformPoint(const mat4_t& A, const vec3_t& p) {
    return vec3(A * vec4(p, 1.f));
}

vec3_t transformVector(const mat4_t& A, const vec3_t& v) {
    return vec3(A * vec4(v, 0.f));
}

mat4_t transpose(const mat4_t& A) {
    mat4_t B;
    for (uint32 i = 0; i < 4; ++i)
        for (uint32 j = 0; j < 4; ++j)
            B(i, j) = A[j][i];
    return B;
}

float determinant(const mat4_t& A) {
    UNUSED(A);
    FATAL_ERR("not yet implemented");
}

mat4_t inverse(const mat4_t& A) {
    // float SubFactor00 = A[2][2] * A[3][3] - A[3][2] * A[2][3];
    // float SubFactor01 = A[2][1] * A[3][3] - A[3][1] * A[2][3];
    // float SubFactor02 = A[2][1] * A[3][2] - A[3][1] * A[2][2];
    // float SubFactor03 = A[2][0] * A[3][3] - A[3][0] * A[2][3];
    // float SubFactor04 = A[2][0] * A[3][2] - A[3][0] * A[2][2];
    // float SubFactor05 = A[2][0] * A[3][1] - A[3][0] * A[2][1];
    // float SubFactor06 = A[1][2] * A[3][3] - A[3][2] * A[1][3];
    // float SubFactor07 = A[1][1] * A[3][3] - A[3][1] * A[1][3];
    // float SubFactor08 = A[1][1] * A[3][2] - A[3][1] * A[1][2];
    // float SubFactor09 = A[1][0] * A[3][3] - A[3][0] * A[1][3];
    // float SubFactor10 = A[1][0] * A[3][2] - A[3][0] * A[1][2];
    // float SubFactor11 = A[1][1] * A[3][3] - A[3][1] * A[1][3];
    // float SubFactor12 = A[1][0] * A[3][1] - A[3][0] * A[1][1];
    // float SubFactor13 = A[1][2] * A[2][3] - A[2][2] * A[1][3];
    // float SubFactor14 = A[1][1] * A[2][3] - A[2][1] * A[1][3];
    // float SubFactor15 = A[1][1] * A[2][2] - A[2][1] * A[1][2];
    // float SubFactor16 = A[1][0] * A[2][3] - A[2][0] * A[1][3];
    // float SubFactor17 = A[1][0] * A[2][2] - A[2][0] * A[1][2];
    // float SubFactor18 = A[1][0] * A[2][1] - A[2][0] * A[1][1];

    // mat4_t inv = mat4(vec4(
    //     + A[1][1] * SubFactor00 - A[1][2] * SubFactor01 + A[1][3] * SubFactor02,
    //     - A[1][0] * SubFactor00 + A[1][2] * SubFactor03 - A[1][3] * SubFactor04,
    //     + A[1][0] * SubFactor01 - A[1][1] * SubFactor03 + A[1][3] * SubFactor05,
    //     - A[1][0] * SubFactor02 + A[1][1] * SubFactor04 - A[1][2] * SubFactor05), vec4(

    //     - A[0][1] * SubFactor00 + A[0][2] * SubFactor01 - A[0][3] * SubFactor02,
    //     + A[0][0] * SubFactor00 - A[0][2] * SubFactor03 + A[0][3] * SubFactor04,
    //     - A[0][0] * SubFactor01 + A[0][1] * SubFactor03 - A[0][3] * SubFactor05,
    //     + A[0][0] * SubFactor02 - A[0][1] * SubFactor04 + A[0][2] * SubFactor05), vec4(

    //     + A[0][1] * SubFactor06 - A[0][2] * SubFactor07 + A[0][3] * SubFactor08,
    //     - A[0][0] * SubFactor06 + A[0][2] * SubFactor09 - A[0][3] * SubFactor10,
    //     + A[0][0] * SubFactor11 - A[0][1] * SubFactor09 + A[0][3] * SubFactor12,
    //     - A[0][0] * SubFactor08 + A[0][1] * SubFactor10 - A[0][2] * SubFactor12), vec4(

    //     - A[0][1] * SubFactor13 + A[0][2] * SubFactor14 - A[0][3] * SubFactor15,
    //     + A[0][0] * SubFactor13 - A[0][2] * SubFactor16 + A[0][3] * SubFactor17,
    //     - A[0][0] * SubFactor14 + A[0][1] * SubFactor16 - A[0][3] * SubFactor18,
    //     + A[0][0] * SubFactor15 - A[0][1] * SubFactor17 + A[0][2] * SubFactor18));

    // float det = 
    //     + A[0][0] * inv[0][0] 
    //     + A[0][1] * inv[1][0] 
    //     + A[0][2] * inv[2][0] 
    //     + A[0][3] * inv[3][0];

    // inv /= det;

    const float *m = A.components;
    mat4_t Ainv;
    float *inv = Ainv.components;

    inv[0] =   m[5]*m[10]*m[15] - m[5]*m[11]*m[14] - m[9]*m[6]*m[15]
        + m[9]*m[7]*m[14] + m[13]*m[6]*m[11] - m[13]*m[7]*m[10];
    inv[4] =  -m[4]*m[10]*m[15] + m[4]*m[11]*m[14] + m[8]*m[6]*m[15]
        - m[8]*m[7]*m[14] - m[12]*m[6]*m[11] + m[12]*m[7]*m[10];
    inv[8] =   m[4]*m[9]*m[15] - m[4]*m[11]*m[13] - m[8]*m[5]*m[15]
        + m[8]*m[7]*m[13] + m[12]*m[5]*m[11] - m[12]*m[7]*m[9];
    inv[12] = -m[4]*m[9]*m[14] + m[4]*m[10]*m[13] + m[8]*m[5]*m[14]
        - m[8]*m[6]*m[13] - m[12]*m[5]*m[10] + m[12]*m[6]*m[9];
    inv[1] =  -m[1]*m[10]*m[15] + m[1]*m[11]*m[14] + m[9]*m[2]*m[15]
        - m[9]*m[3]*m[14] - m[13]*m[2]*m[11] + m[13]*m[3]*m[10];
    inv[5] =   m[0]*m[10]*m[15] - m[0]*m[11]*m[14] - m[8]*m[2]*m[15]
        + m[8]*m[3]*m[14] + m[12]*m[2]*m[11] - m[12]*m[3]*m[10];
    inv[9] =  -m[0]*m[9]*m[15] + m[0]*m[11]*m[13] + m[8]*m[1]*m[15]
        - m[8]*m[3]*m[13] - m[12]*m[1]*m[11] + m[12]*m[3]*m[9];
    inv[13] =  m[0]*m[9]*m[14] - m[0]*m[10]*m[13] - m[8]*m[1]*m[14]
        + m[8]*m[2]*m[13] + m[12]*m[1]*m[10] - m[12]*m[2]*m[9];
    inv[2] =   m[1]*m[6]*m[15] - m[1]*m[7]*m[14] - m[5]*m[2]*m[15]
        + m[5]*m[3]*m[14] + m[13]*m[2]*m[7] - m[13]*m[3]*m[6];
    inv[6] =  -m[0]*m[6]*m[15] + m[0]*m[7]*m[14] + m[4]*m[2]*m[15]
        - m[4]*m[3]*m[14] - m[12]*m[2]*m[7] + m[12]*m[3]*m[6];
    inv[10] =  m[0]*m[5]*m[15] - m[0]*m[7]*m[13] - m[4]*m[1]*m[15]
        + m[4]*m[3]*m[13] + m[12]*m[1]*m[7] - m[12]*m[3]*m[5];
    inv[14] = -m[0]*m[5]*m[14] + m[0]*m[6]*m[13] + m[4]*m[1]*m[14]
        - m[4]*m[2]*m[13] - m[12]*m[1]*m[6] + m[12]*m[2]*m[5];
    inv[3] =  -m[1]*m[6]*m[11] + m[1]*m[7]*m[10] + m[5]*m[2]*m[11]
        - m[5]*m[3]*m[10] - m[9]*m[2]*m[7] + m[9]*m[3]*m[6];
    inv[7] =   m[0]*m[6]*m[11] - m[0]*m[7]*m[10] - m[4]*m[2]*m[11]
        + m[4]*m[3]*m[10] + m[8]*m[2]*m[7] - m[8]*m[3]*m[6];
    inv[11] = -m[0]*m[5]*m[11] + m[0]*m[7]*m[9] + m[4]*m[1]*m[11]
        - m[4]*m[3]*m[9] - m[8]*m[1]*m[7] + m[8]*m[3]*m[5];
    inv[15] =  m[0]*m[5]*m[10] - m[0]*m[6]*m[9] - m[4]*m[1]*m[10]
        + m[4]*m[2]*m[9] + m[8]*m[1]*m[6] - m[8]*m[2]*m[5];

    float det = m[0]*inv[0] + m[1]*inv[4] + m[2]*inv[8] + m[3]*inv[12];
    if (det == 0.f) {
        ERROR_ONCE("matrix has no inverse");
        return mat4(0.f);
    }

    det = 1.0 / det;
    Ainv *= det;

    ASSERT(equal(A * Ainv, mat4()));
    
    return Ainv;
}

vec4_t transposedMult(const mat4_t& AT, const vec4_t& v) {
    return vec4(dot(v, AT[0]), dot(v, AT[1]),
                dot(v, AT[2]), dot(v, AT[3]));
}

bool equal(const mat4_t& A, const mat4_t& B, float epsi) {
    return equal(A[0], B[0], epsi) && equal(A[1], B[1], epsi) &&
        equal(A[2], B[2], epsi) && equal(A[3], B[3], epsi);
}

MATH_END_NAMESPACE

namespace math {

MATH_INLINE_SPEC vec4_t mat4_t::operator[](unsigned long i) const {
    return columns[i];
}

MATH_INLINE_SPEC vec4_t& mat4_t::operator[](unsigned long i) {
    return columns[i];
}

MATH_INLINE_SPEC float mat4_t::operator()(unsigned long i, unsigned long j) const {
    return components[i * 4 + j];
}


MATH_INLINE_SPEC float& mat4_t::operator()(unsigned long i, unsigned long j) {
    return components[i * 4 + j];
}

} // namespace math

#endif
