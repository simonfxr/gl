#ifndef MAT2_TYPE_HPP
#define MAT2_TYPE_HPP

#include "math/real/type.hpp"
#include "math/vec2/type.hpp"

namespace math {

struct glmat2;

struct mat2_t {
    static const defs::size column_size = 2;
    typedef vec2_t column_type;
    typedef column_type::component_type component_type;
    static const defs::size size = column_size * column_type::size;
    static const defs::size padded_size = column_size * column_type::padded_size;
    typedef component_type buffer[size];
    typedef glmat2 gl;

    union {
        column_type columns[column_size];
        component_type components[padded_size];
    };

    MATH_FUNC const column_type& operator[](defs::index) const PURE_FUNC;
    MATH_FUNC column_type& operator[](defs::index) MUT_FUNC;
    MATH_FUNC component_type operator()(defs::index, defs::index) const PURE_FUNC;
    MATH_FUNC component_type& operator()(defs::index, defs::index) MUT_FUNC;
};

typedef mat2_t aligned_mat2_t ATTRS(ATTR_ALIGNED(16));

} // namespace math

#endif

