#ifndef MATH_MAT2_HPP
#define MATH_MAT2_HPP

#include "math/real.hpp"
#include "math/vec2.hpp"

namespace math {

struct glmat2;

struct mat2_t
{
    static const size_t column_size = 2;
    typedef vec2_t column_type;
    typedef column_type::component_type component_type;
    static const size_t size = column_size * column_type::size;
    static const size_t padded_size = column_size * column_type::padded_size;
    typedef component_type buffer[size];
    typedef glmat2 gl;

    union
    {
        column_type columns[column_size];
        component_type components[padded_size];
    };

    constexpr mat2_t() : columns{} {}

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

typedef mat2_t HU_ALIGN(16) aligned_mat2_t;

inline constexpr mat2_t
mat2(const vec2_t &c1, const vec2_t &c2)
{
    mat2_t A;
    A[0] = c1;
    A[1] = c2;
    return A;
}

inline constexpr mat2_t
mat2()
{
    return mat2(vec2(1, 0), vec2(0, 1));
}

inline constexpr mat2_t
mat2(real x)
{
    return mat2(vec2(x), vec2(x));
}

inline constexpr mat2_t
mat2(const mat2_t::buffer mat)
{
    mat2_t A;
    A[0] = vec2(&mat[0]);
    A[1] = vec2(&mat[2]);
    return A;
}

inline constexpr void
load(mat2_t::buffer buf, const mat2_t &A)
{
    load(&buf[0], A[0]);
    load(&buf[2], A[1]);
}

} // namespace math

#endif
