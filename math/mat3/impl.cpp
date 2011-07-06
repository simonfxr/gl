#include "math/defs.hpp"

#if defined(MATH_MAT3_INLINE) || !defined(MATH_INLINE)

#include "math/mat3/defns.hpp"
#include "math/vec3.hpp"
#include "math/math.hpp"

#include "error/error.hpp"

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

mat3_t mat3(const mat4_t& A) {
    return mat3(vec3(A[0]), vec3(A[1]), vec3(A[2]));
}

mat3_t mat3(const quat_t& q) {
    float w = q.a, x = q.b, y = q.c, z = q.d;
    float xx = x * x, yy = y * y, zz = z * z;
    
    return mat3(vec3(1 - 2 * yy - 2 * zz, 2 * x * y - 2 * w * z, 2 * x * z + 2 * w * y),
                vec3(2 * x * y + 2 * w * z, 1 - 2 * xx - 2 * zz, 2 * y * z - 2 * w * x),
                vec3(2 * x * z - 2 * w * y, 2 * y * z + 2 * w * x, 1 - 2 * xx - 2 * yy));
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
    return A * math::recip(x);
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

float determinant(const mat3_t& A) {
    const mat3_t AT = transpose(A);
    struct { const float *data; } m = { AT.components };

    float t4 = m.data[0]*m.data[4];
    float t6 = m.data[0]*m.data[5];
    float t8 = m.data[1]*m.data[3];
    float t10 = m.data[2]*m.data[3];
    float t12 = m.data[1]*m.data[6];
    float t14 = m.data[2]*m.data[6];
    // Calculate the determinant.
    float t16 = (t4*m.data[8] - t6*m.data[7] - t8*m.data[8] +
                 t10*m.data[7] + t12*m.data[5] - t14*m.data[4]);
    return t16;
}

mat3_t inverse(const mat3_t& A) {
    const mat3_t AT = transpose(A);
    mat3_t B;
    struct { const float *data; } m = { AT.components };
    float *data = B.components;

    float t4 = m.data[0]*m.data[4];
    float t6 = m.data[0]*m.data[5];
    float t8 = m.data[1]*m.data[3];
    float t10 = m.data[2]*m.data[3];
    float t12 = m.data[1]*m.data[6];
    float t14 = m.data[2]*m.data[6];
    // Calculate the determinant.
    float t16 = (t4*m.data[8] - t6*m.data[7] - t8*m.data[8] +
                 t10*m.data[7] + t12*m.data[5] - t14*m.data[4]);
    
    // Make sure the determinant is non-zero.
    if (t16 == 0.0f) return mat3(0.f);
    float t17 = 1/t16;
    data[0] = (m.data[4]*m.data[8]-m.data[5]*m.data[7])*t17; 
    data[1] = -(m.data[1]*m.data[8]-m.data[2]*m.data[7])*t17;
    data[2] = (m.data[1]*m.data[5]-m.data[2]*m.data[4])*t17; 
    data[3] = -(m.data[3]*m.data[8]-m.data[5]*m.data[6])*t17;
    data[4] = (m.data[0]*m.data[8]-t14)*t17;                 
    data[5] = -(t6-t10)*t17;                                 
    data[6] = (m.data[3]*m.data[7]-m.data[4]*m.data[6])*t17; 
    data[7] = -(m.data[0]*m.data[7]-t12)*t17;                
    data[8] = (t4-t8)*t17;

    mat3_t Ainv = transpose(B);

    ASSERT(equal(A * Ainv, mat3()));

    return Ainv;
}

mat3_t orthonormalBasis(const mat3_t& A) {
    direction3_t u = normalize(A[0]);
    direction3_t v = normalize(A[1] - projectAlong(A[1], u));
    return mat3(u, v, cross(u, v));
}

vec3_t transform(const mat3_t& A, const vec3_t& v) {
    return A * v;
}

point3_t transformPoint(const mat3_t& A, const point3_t& v) {
    return transform(A, v);
}

vec3_t transformVector(const mat3_t& A, const vec3_t& v) {
    return transform(A, v);
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

MATH_INLINE_SPEC const vec3_t& mat3_t::operator[](unsigned long i) const {
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

