#ifndef MAT2_TYPE_HPP
#define MAT2_TYPE_HPP

#include "math/real/type.hpp"
#include "math/vec2/type.hpp"

namespace math {

struct glmat2;

struct mat2_t
{
    static const defs::size_t column_size = 2;
    typedef vec2_t column_type;
    typedef column_type::component_type component_type;
    static const defs::size_t size_t = column_size * column_type::size_t;
    static const defs::size_t padded_size =
      column_size * column_type::padded_size;
    typedef component_type buffer[size_t];
    typedef glmat2 gl;

    union
    {
        column_type columns[column_size];
        component_type components[padded_size];
    };

    constexpr mat2_t() : columns{} {}

    constexpr MATH_FUNC const column_type &operator[](
      defs::index_t) const PURE_FUNC;
    constexpr MATH_FUNC column_type &operator[](defs::index_t) MUT_FUNC;
    constexpr MATH_FUNC component_type operator()(defs::index_t,
                                                  defs::index_t) const PURE_FUNC;
    constexpr MATH_FUNC component_type &operator()(defs::index_t,
                                                   defs::index_t) MUT_FUNC;
};

typedef mat2_t aligned_mat2_t ATTRS(ATTR_ALIGNED(16));

} // namespace math

#endif
