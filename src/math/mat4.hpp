#ifndef MATH_MAT4_HPP
#define MATH_MAT4_HPP

#include "math/mat3.hpp"
#include "math/real.hpp"
#include "math/vec4.hpp"

namespace math {

struct glmat4;

struct mat4_t
{
    static const size_t column_size = 4;
    typedef vec4_t column_type;
    typedef column_type::component_type component_type;
    static const size_t size = column_size * column_type::size;
    static const size_t padded_size = column_size * column_type::padded_size;
    typedef component_type buffer[size];
    typedef glmat4 gl;

    union
    {
        column_type columns[column_size];
        component_type components[padded_size];
    };

    constexpr mat4_t() : columns{} {}

    constexpr inline const column_type &operator[](size_t i) const
    {
        return columns[i];
    }

    constexpr inline column_type &operator[](size_t i) { return columns[i]; }

    constexpr inline component_type operator()(size_t i, size_t j) const
    {
        return columns[i][j];
    }

    constexpr inline component_type &operator()(size_t i, size_t j)
    {
        return columns[i][j];
    }
};

typedef mat4_t HU_ALIGN(16) aligned_mat4_t;

} // namespace math

namespace math {

inline constexpr mat4_t
mat4(const vec4_t &c1, const vec4_t &c2, const vec4_t &c3, const vec4_t &c4)
{
    mat4_t A;
    A[0] = c1;
    A[1] = c2;
    A[2] = c3;
    A[3] = c4;
    return A;
}

inline constexpr mat4_t
mat4()
{
    return mat4(
      vec4(1, 0, 0, 0), vec4(0, 1, 0, 0), vec4(0, 0, 1, 0), vec4(0, 0, 0, 1));
}

inline constexpr mat4_t
mat4(real x)
{
    return mat4(vec4(x), vec4(x), vec4(x), vec4(x));
}

inline constexpr mat4_t
mat4(const real mat[16])
{
    mat4_t A;
    for (uint32_t i = 0; i < 16; ++i)
        A.components[i] = mat[i];
    return A;
}

inline constexpr mat4_t
mat4(const mat3_t &m)
{
    return mat4(vec4(m[0], 0), vec4(m[1], 0), vec4(m[2], 0), vec4(0, 0, 0, 1));
}

constexpr mat3_t
mat3(const mat4_t &A)
{
    return mat3(vec3(A[0]), vec3(A[1]), vec3(A[2]));
}

inline constexpr void
load(mat4_t::buffer b, const mat4_t &m)
{
    load(&b[0], m[0]);
    load(&b[4], m[1]);
    load(&b[8], m[2]);
    load(&b[12], m[3]);
}

inline constexpr mat4_t
operator+(const mat4_t &A, const mat4_t &B)
{
    return mat4(A[0] + B[0], A[1] + B[1], A[2] + B[2], A[3] + B[3]);
}

inline constexpr mat4_t
operator-(const mat4_t &A, const mat4_t &B)
{
    return mat4(A[0] - B[0], A[1] - B[1], A[2] - B[2], A[3] - B[3]);
}

inline constexpr mat4_t operator*(const mat4_t &A, const mat4_t &B)
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

inline constexpr vec4_t operator*(const mat4_t &A, const vec4_t &v)
{
    return v[0] * A[0] + v[1] * A[1] + v[2] * A[2] + v[3] * A[3];
}

inline constexpr mat4_t operator*(const mat4_t &A, real x)
{
    return mat4(A[0] * x, A[1] * x, A[2] * x, A[3] * x);
}

inline constexpr mat4_t operator*(real x, const mat4_t &A)
{
    return A * x;
}

inline constexpr mat4_t
operator/(const mat4_t &A, real x)
{
    return A * math::recip(x);
}

inline constexpr mat4_t &
operator+=(mat4_t &A, const mat4_t &B)
{
    return A = A + B;
}

inline constexpr mat4_t &
operator-=(mat4_t &A, const mat4_t &B)
{
    return A = A - B;
}

inline constexpr mat4_t &
operator*=(mat4_t &A, real x)
{
    return A = A * x;
}

inline constexpr mat4_t &
operator*=(mat4_t &A, const mat4_t &B)
{
    return A = A * B;
}

inline constexpr mat4_t &
operator/=(mat4_t &A, real x)
{
    return A = A / x;
}

inline constexpr vec4_t
transform(const mat4_t &A, const vec4_t &v)
{
    return A * v;
}

inline constexpr vec3_t
transformPoint(const mat4_t &A, const vec3_t &p)
{
    return vec3(A * vec4(p, real(1)));
}

inline constexpr vec3_t
transformVector(const mat4_t &A, const vec3_t &v)
{
    return vec3(A * vec4(v, real(0)));
}

inline constexpr mat4_t
transpose(const mat4_t &A)
{
    mat4_t B;
    for (size_t i = 0; i < 4; ++i)
        for (size_t j = 0; j < 4; ++j)
            B(i, j) = A(j, i);
    return B;
}

inline mat4_t
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
        return mat4(real(0));
    }

    det = recip(det);
    Ainv *= det;

    return Ainv;
}

inline constexpr vec4_t
transposedMult(const mat4_t &AT, const vec4_t &v)
{
    return vec4(dot(v, AT[0]), dot(v, AT[1]), dot(v, AT[2]), dot(v, AT[3]));
}

inline bool
equal(const mat4_t &A, const mat4_t &B, real epsi)
{
    return equal(A[0], B[0], epsi) && equal(A[1], B[1], epsi) &&
           equal(A[2], B[2], epsi) && equal(A[3], B[3], epsi);
}
} // namespace math

#endif
