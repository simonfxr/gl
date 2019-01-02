#ifndef MAT4_TYPE_HPP
#define MAT4_TYPE_HPP

#include "math/real/type.hpp"
#include "math/vec4/type.hpp"

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

    constexpr MATH_FUNC const column_type &operator[](size_t) const PURE_FUNC;
    constexpr MATH_FUNC column_type &operator[](size_t) MUT_FUNC;
    constexpr MATH_FUNC component_type operator()(size_t,
                                                  size_t) const PURE_FUNC;
    constexpr MATH_FUNC component_type &operator()(size_t, size_t) MUT_FUNC;
};

typedef mat4_t aligned_mat4_t ATTRS(ATTR_ALIGNED(16));

} // namespace math

#endif
