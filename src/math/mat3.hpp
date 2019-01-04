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
    return mat3_t::make(c1, c2, c3);
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

} // namespace math

#endif
