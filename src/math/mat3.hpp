#ifndef MATH_MAT3_HPP
#define MATH_MAT3_HPP

#include "math/real.hpp"
#include "math/vec3.hpp"

namespace math {

struct glmat3;

struct mat3_t
{
    static const size_t column_size = 3;
    typedef vec3_t column_type;
    typedef column_type::component_type component_type;
    static const size_t size = column_size * column_type::size;
    static const size_t padded_size = column_size * column_type::padded_size;
    typedef component_type buffer[size];
    typedef glmat3 gl;

    column_type columns[column_size];

    constexpr mat3_t() : columns{} {}

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

typedef mat3_t HU_ALIGN(16) aligned_mat3_t;

inline constexpr mat3_t
mat3(const vec3_t &c1, const vec3_t &c2, const vec3_t &c3)
{
    mat3_t A;
    A[0] = c1;
    A[1] = c2;
    A[2] = c3;
    return A;
}

inline constexpr mat3_t
mat3()
{
    return mat3(vec3(1, 0, 0), vec3(0, 1, 0), vec3(0, 0, 1));
}

inline constexpr mat3_t
mat3(real x)
{
    return mat3(vec3(x), vec3(x), vec3(x));
}

inline constexpr mat3_t
mat3(const mat3_t::buffer mat)
{
    mat3_t A;
    A[0] = vec3(&mat[0]);
    A[1] = vec3(&mat[3]);
    A[2] = vec3(&mat[6]);
    return A;
}

inline constexpr void
load(mat3_t::buffer b, const mat3_t &m)
{
    load(&b[0], m[0]);
    load(&b[3], m[1]);
    load(&b[6], m[2]);
}

inline constexpr mat3_t
operator+(const mat3_t &A, const mat3_t &B)
{
    return mat3(A[0] + B[0], A[1] + B[1], A[2] + B[2]);
}

inline constexpr mat3_t
operator-(const mat3_t &A, const mat3_t &B)
{
    return mat3(A[0] - B[0], A[1] - B[1], A[2] - B[2]);
}

inline constexpr mat3_t
transpose(const mat3_t &A)
{
    mat3_t B;
    for (size_t i = 0; i < 3; ++i)
        for (size_t j = 0; j < 3; ++j)
            B(i, j) = A[j][i];
    return B;
}

inline constexpr mat3_t operator*(const mat3_t &A, const mat3_t &B)
{
    mat3_t AT = transpose(A);
    mat3_t C;
    for (size_t i = 0; i < 3; ++i)
        for (size_t j = 0; j < 3; ++j)
            C(i, j) = dot(AT[j], B[i]);
    return C;
}

inline constexpr vec3_t operator*(const mat3_t &A, const vec3_t &v)
{
    return v[0] * A[0] + v[1] * A[1] + v[2] * A[2];
    // mat3_t AT = transpose(A);
    // return vec3(dot(v, AT[0]), dot(v, AT[1]), dot(v, AT[2]));
}

inline constexpr mat3_t operator*(const mat3_t &A, real x)
{
    return mat3(A[0] * x, A[1] * x, A[2] * x);
}

inline constexpr mat3_t operator*(real x, const mat3_t &A)
{
    return A * x;
}

inline constexpr mat3_t
operator/(const mat3_t &A, real x)
{
    return A * math::recip(x);
}

inline constexpr mat3_t &
operator+=(mat3_t &A, const mat3_t &B)
{
    return A = A + B;
}

inline constexpr mat3_t &
operator-=(mat3_t &A, const mat3_t &B)
{
    return A = A - B;
}

inline constexpr mat3_t &
operator*=(mat3_t &A, real x)
{
    return A = A * x;
}

inline constexpr mat3_t &
operator*=(mat3_t &A, const mat3_t &B)
{
    return A = A * B;
}

inline constexpr mat3_t &
operator/=(mat3_t &A, real x)
{
    return A = A / x;
}

inline constexpr real
determinant(const mat3_t &A)
{
    const mat3_t AT = transpose(A);
    struct
    {
        mat3_t::buffer data;
    } m{};
    load(m.data, AT);

    real t4 = m.data[0] * m.data[4];
    real t6 = m.data[0] * m.data[5];
    real t8 = m.data[1] * m.data[3];
    real t10 = m.data[2] * m.data[3];
    real t12 = m.data[1] * m.data[6];
    real t14 = m.data[2] * m.data[6];
    // Calculate the determinant.
    real t16 = (t4 * m.data[8] - t6 * m.data[7] - t8 * m.data[8] +
                t10 * m.data[7] + t12 * m.data[5] - t14 * m.data[4]);
    return t16;
}

inline mat3_t
inverse(const mat3_t &A)
{
    const mat3_t AT = transpose(A);
    struct
    {
        mat3_t::buffer data;
    } m;
    load(m.data, AT);
    mat3_t::buffer data{};

    real t4 = m.data[0] * m.data[4];
    real t6 = m.data[0] * m.data[5];
    real t8 = m.data[1] * m.data[3];
    real t10 = m.data[2] * m.data[3];
    real t12 = m.data[1] * m.data[6];
    real t14 = m.data[2] * m.data[6];
    // Calculate the determinant.
    real t16 = (t4 * m.data[8] - t6 * m.data[7] - t8 * m.data[8] +
                t10 * m.data[7] + t12 * m.data[5] - t14 * m.data[4]);

    // Make sure the determinant is non-zero.
    if (t16 == real(0))
        return mat3(real(0));
    real t17 = 1 / t16;
    data[0] = (m.data[4] * m.data[8] - m.data[5] * m.data[7]) * t17;
    data[1] = -(m.data[1] * m.data[8] - m.data[2] * m.data[7]) * t17;
    data[2] = (m.data[1] * m.data[5] - m.data[2] * m.data[4]) * t17;
    data[3] = -(m.data[3] * m.data[8] - m.data[5] * m.data[6]) * t17;
    data[4] = (m.data[0] * m.data[8] - t14) * t17;
    data[5] = -(t6 - t10) * t17;
    data[6] = (m.data[3] * m.data[7] - m.data[4] * m.data[6]) * t17;
    data[7] = -(m.data[0] * m.data[7] - t12) * t17;
    data[8] = (t4 - t8) * t17;

    mat3_t B = mat3(data);
    mat3_t Ainv = transpose(B);

    return Ainv;
}

inline mat3_t
orthonormalBasis(const mat3_t &A)
{
    direction3_t u = normalize(A[0]);
    direction3_t v = normalize(A[1] - projectAlong(A[1], u));
    return mat3(u, v, cross(u, v));
}

inline constexpr vec3_t
transform(const mat3_t &A, const vec3_t &v)
{
    return A * v;
}

inline constexpr point3_t
transformPoint(const mat3_t &A, const point3_t &v)
{
    return transform(A, v);
}

inline constexpr vec3_t
transformVector(const mat3_t &A, const vec3_t &v)
{
    return transform(A, v);
}

inline bool
equal(const mat3_t &A, const mat3_t &B, real epsi)
{
    return equal(A[0], B[0], epsi) && equal(A[1], B[1], epsi) &&
           equal(A[2], B[2], epsi);
}

inline mat3_t
coordinateSystem(const vec3_t &a)
{
    vec3_t aa = abs(a);
    vec3_t b{};

    if (aa[0] > aa[1] && aa[0] > aa[2])
        b = vec3(-a[2], 0, a[0]);
    else if (aa[1] > aa[2])
        b = vec3(a[1], -a[0], 0);
    else
        b = vec3(0, a[2], -a[1]);

    return mat3(a, b, cross(a, b));
}
} // namespace math

#endif
