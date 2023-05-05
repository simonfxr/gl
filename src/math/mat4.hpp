#ifndef MATH_MAT4_HPP
#define MATH_MAT4_HPP

#include "math/mat.hpp"
#include "math/mat3.hpp"

namespace math {

using mat4_t = mat<real, 4>;

constexpr mat4_t
mat4()
{
    return mat4_t::identity();
}

constexpr mat4_t
mat4(real x)
{
    return mat4_t::broadcast(x);
}

template<typename T>
constexpr mat4_t
mat4(const mat<T, 4> &A)
{
    return mat4_t::convert(A);
}

constexpr mat4_t
mat4(const vec<real, 4> &c1,
     const vec<real, 4> &c2,
     const vec<real, 4> &c3,
     const vec<real, 4> &c4)
{
    return { c1, c2, c3, c4 };
}

constexpr mat4_t
mat4(const mat<real, 3> &A)
{
    auto B = mat4();
    for (size_t i = 0; i < 3; ++i)
        for (size_t j = 0; j < 3; ++j)
            B[i][j] = A[i][j];
    return B;
}

template<typename T, typename U>
constexpr auto
transformPoint(const mat<T, 4> &A, const vec<U, 3> &v)
{
    auto u = A(vec<U, 4>(bl::in_place_t{}, v[0], v[1], v[2], U(1)));
    return vec<typename decltype(u)::scalar_type, 3>(
      bl::in_place_t{}, u[0], u[1], u[2]);
}

template<typename T, typename U>
constexpr auto
transformVector(const mat<T, 4> &A, const vec<U, 3> &v)
{
    auto u = A(vec<U, 4>(bl::in_place_t{}, v[0], v[1], v[2], U(0)));
    return vec<typename decltype(u)::scalar_type, 3>(
      bl::in_place_t{}, u[0], u[1], u[2]);
}

} // namespace math

#endif
