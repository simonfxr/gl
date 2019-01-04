#ifndef MATH_MAT2_HPP
#define MATH_MAT2_HPP

#include "math/genmat.hpp"

namespace math {

using mat2_t = genmat<real, 2>;

constexpr mat2_t
mat2()
{
    return mat2_t::identity();
}

constexpr mat2_t
mat2(real x)
{
    return mat2_t::fill(x);
}

template<typename T>
constexpr mat2_t
mat2(const genmat<T, 2> &A)
{
    return mat2_t::convert(A);
}

constexpr mat2_t
mat2(mat2_t::buffer b)
{
    return mat2_t::load(b);
}

constexpr mat2_t
mat2(const genvec<real, 2> &c1, const genvec<real, 2> &c2)
{
    return mat2_t::make(c1, c2);
}

constexpr mat2_t
mat2(const genmat<real, 3> &A)
{
    mat2_t B{};
    for (size_t i = 0; i < 2; ++i)
        for (size_t j = 0; j < 2; ++j)
            B[i][j] = A[i][j];
    return B;
}

} // namespace math

#endif
