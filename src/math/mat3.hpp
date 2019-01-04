#ifndef MATH_MAT3_HPP
#define MATH_MAT3_HPP

#include "math/genmat.hpp"

namespace math {
using mat3_t = genmat<real, 3>;

using aligned_mat3_t = HU_ALIGN(16) mat3_t;

constexpr mat3_t
mat3()
{
    return mat3_t::identity();
}

constexpr mat3_t
mat3(real x)
{
    return mat3_t::fill(x);
}

template<typename T>
constexpr mat3_t
mat3(const genmat<T, 3> &A)
{
    return mat3_t::convert(A);
}

constexpr mat3_t
mat3(mat3_t::buffer b)
{
    return mat3_t::load(b);
}

constexpr mat3_t
mat3(const genvec<real, 3> &c1,
     const genvec<real, 3> &c2,
     const genvec<real, 3> &c3)
{
    return { c1, c2, c3 };
}

constexpr mat3_t
mat3(const genmat<real, 4> &A)
{
    mat3_t B{};
    for (size_t i = 0; i < 3; ++i)
        for (size_t j = 0; j < 3; ++j)
            B[i][j] = A[i][j];
    return B;
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
    auto u = normalize(A[0]);
    auto v = normalize(A[1] - projectAlong(A[1], u));
    return mat3(u, v, cross(u, v));
}

inline constexpr mat3_t
coordinateSystem(const genvec<real, 3> &a)
{
    using V3 = genvec<real, 3>;
    auto aa = abs(a);
    V3 b{};

    if (aa[0] > aa[1] && aa[0] > aa[2])
        b = V3{ -a[2], 0, a[0] };
    else if (aa[1] > aa[2])
        b = V3{ a[1], -a[0], 0 };
    else
        b = V3{ 0, a[2], -a[1] };

    return mat3(a, b, cross(a, b));
}
} // namespace math

#endif
