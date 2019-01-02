#ifndef MAT3_TYPE_HPP
#define MAT3_TYPE_HPP

#include "math/real/type.hpp"
#include "math/vec3/type.hpp"

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

    union
    {
        column_type columns[column_size];
        component_type components[padded_size];
    };

    constexpr mat3_t() : columns{} {}

    constexpr MATH_FUNC const column_type &operator[](size_t) const PURE_FUNC;
    constexpr MATH_FUNC column_type &operator[](size_t) MUT_FUNC;
    constexpr MATH_FUNC component_type operator()(size_t,
                                                  size_t) const PURE_FUNC;
    constexpr MATH_FUNC component_type &operator()(size_t, size_t) MUT_FUNC;
};

typedef mat3_t aligned_mat3_t ATTRS(ATTR_ALIGNED(16));

} // namespace math

#endif
