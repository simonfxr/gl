#include "math/mat4/defns.hpp"
#include "math/real.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"

#include "err/err.hpp"

namespace math {

constexpr mat4_t
mat4()
{
    return mat4(
      vec4(1, 0, 0, 0), vec4(0, 1, 0, 0), vec4(0, 0, 1, 0), vec4(0, 0, 0, 1));
}

constexpr mat4_t
mat4(real x)
{
    return mat4(vec4(x), vec4(x), vec4(x), vec4(x));
}

constexpr mat4_t
mat4(const real mat[16])
{
    mat4_t A;
    for (uint32_t i = 0; i < 16; ++i)
        A.components[i] = mat[i];
    return A;
}

constexpr mat4_t
mat4(const mat3_t &m)
{
    return mat4(vec4(m[0], 0), vec4(m[1], 0), vec4(m[2], 0), vec4(0, 0, 0, 1));
}

constexpr mat4_t
mat4(const vec4_t &c1, const vec4_t &c2, const vec4_t &c3, const vec4_t &c4)
{
    mat4_t A;
    A[0] = c1;
    A[1] = c2;
    A[2] = c3;
    A[3] = c4;
    return A;
}

void
load(mat4_t::buffer b, const mat4_t &m)
{
    load(&b[0], m[0]);
    load(&b[4], m[1]);
    load(&b[8], m[2]);
    load(&b[12], m[3]);
}

constexpr mat4_t
operator+(const mat4_t &A, const mat4_t &B)
{
    return mat4(A[0] + B[0], A[1] + B[1], A[2] + B[2], A[3] + B[3]);
}

constexpr mat4_t
operator-(const mat4_t &A, const mat4_t &B)
{
    return mat4(A[0] - B[0], A[1] - B[1], A[2] - B[2], A[3] - B[3]);
}

constexpr mat4_t operator*(const mat4_t &A, const mat4_t &B)
{
    mat4_t C;
    // mat4_t AT = transpose(A);
    // for (uint32_t i = 0; i < 4; ++i)
    //     for (uint32_t j = 0; j < 4; ++j)
    //         C(i, j) = dot(AT[j], B[i]);

    for (size_t j = 0; j < 4; ++j) {
        for (size_t i = 0; i < 4; ++i) {
            real c_i_j = real(0);
            for (size_t k = 0; k < 4; ++k)
                c_i_j += A(k, i) * B(j, k);
            C(j, i) = c_i_j;
        }
    }

    return C;
}

constexpr vec4_t operator*(const mat4_t &A, const vec4_t &v)
{
    return v[0] * A[0] + v[1] * A[1] + v[2] * A[2] + v[3] * A[3];
}

constexpr mat4_t operator*(const mat4_t &A, real x)
{
    return mat4(A[0] * x, A[1] * x, A[2] * x, A[3] * x);
}

constexpr mat4_t operator*(real x, const mat4_t &A)
{
    return A * x;
}

constexpr mat4_t
operator/(const mat4_t &A, real x)
{
    return A * math::recip(x);
}

constexpr mat4_t &
operator+=(mat4_t &A, const mat4_t &B)
{
    return A = A + B;
}

constexpr mat4_t &
operator-=(mat4_t &A, const mat4_t &B)
{
    return A = A - B;
}

constexpr mat4_t &
operator*=(mat4_t &A, real x)
{
    return A = A * x;
}

constexpr mat4_t &
operator*=(mat4_t &A, const mat4_t &B)
{
    return A = A * B;
}

constexpr mat4_t &
operator/=(mat4_t &A, real x)
{
    return A = A / x;
}

constexpr vec4_t
transform(const mat4_t &A, const vec4_t &v)
{
    return A * v;
}

constexpr vec3_t
transformPoint(const mat4_t &A, const vec3_t &p)
{
    return vec3(A * vec4(p, real(1)));
}

constexpr vec3_t
transformVector(const mat4_t &A, const vec3_t &v)
{
    return vec3(A * vec4(v, real(0)));
}

constexpr mat4_t
transpose(const mat4_t &A)
{
    mat4_t B;
    for (size_t i = 0; i < 4; ++i)
        for (size_t j = 0; j < 4; ++j)
            B(i, j) = A(j, i);
    return B;
}

real
determinant(const mat4_t &A)
{
    UNUSED(A);
    FATAL_ERR(ERROR_DEFAULT_STREAM, "not yet implemented");
}

mat4_t
inverse(const mat4_t &A)
{
    const real *m = A.components;
    mat4_t Ainv;
    real *inv = Ainv.components;

    inv[0] = m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - m[9] * m[6] * m[15] +
             m[9] * m[7] * m[14] + m[13] * m[6] * m[11] - m[13] * m[7] * m[10];
    inv[4] = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] +
             m[8] * m[6] * m[15] - m[8] * m[7] * m[14] - m[12] * m[6] * m[11] +
             m[12] * m[7] * m[10];
    inv[8] = m[4] * m[9] * m[15] - m[4] * m[11] * m[13] - m[8] * m[5] * m[15] +
             m[8] * m[7] * m[13] + m[12] * m[5] * m[11] - m[12] * m[7] * m[9];
    inv[12] = -m[4] * m[9] * m[14] + m[4] * m[10] * m[13] +
              m[8] * m[5] * m[14] - m[8] * m[6] * m[13] - m[12] * m[5] * m[10] +
              m[12] * m[6] * m[9];
    inv[1] = -m[1] * m[10] * m[15] + m[1] * m[11] * m[14] +
             m[9] * m[2] * m[15] - m[9] * m[3] * m[14] - m[13] * m[2] * m[11] +
             m[13] * m[3] * m[10];
    inv[5] = m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - m[8] * m[2] * m[15] +
             m[8] * m[3] * m[14] + m[12] * m[2] * m[11] - m[12] * m[3] * m[10];
    inv[9] = -m[0] * m[9] * m[15] + m[0] * m[11] * m[13] + m[8] * m[1] * m[15] -
             m[8] * m[3] * m[13] - m[12] * m[1] * m[11] + m[12] * m[3] * m[9];
    inv[13] = m[0] * m[9] * m[14] - m[0] * m[10] * m[13] - m[8] * m[1] * m[14] +
              m[8] * m[2] * m[13] + m[12] * m[1] * m[10] - m[12] * m[2] * m[9];
    inv[2] = m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - m[5] * m[2] * m[15] +
             m[5] * m[3] * m[14] + m[13] * m[2] * m[7] - m[13] * m[3] * m[6];
    inv[6] = -m[0] * m[6] * m[15] + m[0] * m[7] * m[14] + m[4] * m[2] * m[15] -
             m[4] * m[3] * m[14] - m[12] * m[2] * m[7] + m[12] * m[3] * m[6];
    inv[10] = m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[4] * m[1] * m[15] +
              m[4] * m[3] * m[13] + m[12] * m[1] * m[7] - m[12] * m[3] * m[5];
    inv[14] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13] + m[4] * m[1] * m[14] -
              m[4] * m[2] * m[13] - m[12] * m[1] * m[6] + m[12] * m[2] * m[5];
    inv[3] = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11] -
             m[5] * m[3] * m[10] - m[9] * m[2] * m[7] + m[9] * m[3] * m[6];
    inv[7] = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11] +
             m[4] * m[3] * m[10] + m[8] * m[2] * m[7] - m[8] * m[3] * m[6];
    inv[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11] -
              m[4] * m[3] * m[9] - m[8] * m[1] * m[7] + m[8] * m[3] * m[5];
    inv[15] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10] +
              m[4] * m[2] * m[9] + m[8] * m[1] * m[6] - m[8] * m[2] * m[5];

    real det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
    if (det == real(0)) {
        ERR(ERROR_DEFAULT_STREAM, "matrix has no inverse");
        return mat4(real(0));
    }

    det = recip(det);
    Ainv *= det;

    ASSERT(equal(A * Ainv, mat4()));

    return Ainv;
}

constexpr vec4_t
transposedMult(const mat4_t &AT, const vec4_t &v)
{
    return vec4(dot(v, AT[0]), dot(v, AT[1]), dot(v, AT[2]), dot(v, AT[3]));
}

bool
equal(const mat4_t &A, const mat4_t &B, real epsi)
{
    return equal(A[0], B[0], epsi) && equal(A[1], B[1], epsi) &&
           equal(A[2], B[2], epsi) && equal(A[3], B[3], epsi);
}
} // namespace math
