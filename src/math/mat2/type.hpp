#ifndef MAT2_TYPE_HPP
#define MAT2_TYPE_HPP

#include "math/real/type.hpp"
#include "math/vec2/type.hpp"

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

} // namespace math

#endif
