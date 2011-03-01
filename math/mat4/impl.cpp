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
    return A * math::recp(x);
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

vec3_t transformVec(const mat4_t& A, const vec3_t& v) {
    return vec3(A * vec4(v, 0.f));
}

mat4_t transpose(const mat4_t& A) {
    mat4_t B;
    for (uint32 i = 0; i < 4; ++i)
        for (uint32 j = 0; j < 4; ++j)
            B(i, j) = A[j][i];
    return B;
}

mat4_t inverse(const mat4_t& A) {
    UNUSED(A);
    FATAL_ERROR("not yet implemented");
}

vec4_t transposedMult(const mat4_t& AT, const vec4_t& v) {
    return vec4(dot(v, AT[0]), dot(v, AT[1]),
                dot(v, AT[2]), dot(v, AT[3]));
}

MATH_END_NAMESPACE

namespace math {

MATH_INLINE_SPEC vec4_t mat4_t::operator[](unsigned long i) const {
    return columns[i];
}

MATH_INLINE_SPEC vec4_t& mat4_t::operator[](unsigned long i) {
    return columns[i];
}

MATH_INLINE_SPEC float& mat4_t::operator()(unsigned long i, unsigned long j) {
    return components[i * 4 + j];
}

} // namespace math

#endif
