#if defined(MATH_MAT3_INLINE) || !defined(MATH_INLINE)

#include "defs.h"
#include "math/mat3/defns.hpp"
#include "math/vec3.hpp"
#include "math/math.hpp"

#include "glt/utils.hpp"

MATH_BEGIN_NAMESPACE

mat3_t mat3() {
    return mat3(vec3(1.f, 0.f, 0.f),
                vec3(0.f, 1.f, 0.f),
                vec3(0.f, 0.f, 1.f));
}

mat3_t mat3(float x) {
    return mat3(vec3(x), vec3(x), vec3(x));
}

mat3_t mat3(const float mat[9]) {
    mat3_t A;
    for (uint32 i = 0; i < 9; ++i)
        A.components[i] = mat[i];
    return A;
}

mat3_t mat3(const vec3_t& c1, const vec3_t& c2, const vec3_t& c3) {
    mat3_t A; A[0] = c1; A[1] = c2; A[2] = c3; return A;
}

mat3_t operator +(const mat3_t& A, const mat3_t& B) {
    return mat3(A[0] + B[0], A[1] + B[1], A[2] + B[2]);
}

mat3_t operator -(const mat3_t& A, const mat3_t& B) {
    return mat3(A[0] - B[0], A[1] - B[1], A[2] - B[2]);
}

mat3_t operator *(const mat3_t& A, const mat3_t& B) {
    mat3_t AT = transpose(A);
    mat3_t C;
    for (uint32 i = 0; i < 3; ++i)
        for (uint32 j = 0; j < 3; ++j)
            C(i, j) = dot(AT[j], B[i]);
    return C;
}

vec3_t operator *(const mat3_t& A, const vec3_t& v) {
    return v[0] * A[0] + v[1] * A[1] + v[2] * A[2];
    // mat3_t AT = transpose(A);
    // return vec3(dot(v, AT[0]), dot(v, AT[1]), dot(v, AT[2]));
}

mat3_t operator *(const mat3_t& A, float x) {
    return mat3(A[0] * x, A[1] * x, A[2] * x);
}

mat3_t operator *(float x, const mat3_t& A) {
    return A * x;
}

mat3_t operator /(const mat3_t& A, float x) {
    return A * math::recp(x);
}

mat3_t& operator +=(mat3_t& A, const mat3_t& B) {
    return A = A + B;
}

mat3_t& operator -=(mat3_t& A, const mat3_t& B) {
    return A = A - B;
}

mat3_t& operator *=(mat3_t& A, float x) {
    return A = A * x;
}

mat3_t& operator *=(mat3_t& A, const mat3_t& B) {
    return A = A * B;
}

mat3_t& operator /=(mat3_t& A, float x) {
    return A = A / x;
}

vec3_t transform(const mat3_t& A, const vec3_t& v) {
    return A * v;
}

mat3_t inverse(const mat3_t& A) {
    UNUSED(A);
    FATAL_ERROR("not yet implemented");
}

mat3_t transpose(const mat3_t& A) {
    mat3_t B;
    for (uint32 i = 0; i < 3; ++i)
        for (uint32 j = 0; j < 3; ++j)
            B(i, j) = A[j][i];
    return B;
}

bool equal(const mat3_t& A, const mat3_t& B, float epsi) {
    return equal(A[0], B[0], epsi) && equal(A[1], B[1], epsi) && equal(A[2], B[2], epsi);
}

MATH_END_NAMESPACE

namespace math {

MATH_INLINE_SPEC vec3_t mat3_t::operator[](unsigned long i) const {
    return columns[i];
}

MATH_INLINE_SPEC vec3_t& mat3_t::operator[](unsigned long i) {
    return columns[i];
}

MATH_INLINE_SPEC float& mat3_t::operator()(unsigned long i, unsigned long j) {
    return components[i * 3 + j];
}

} // namespace math

#endif
