#ifndef MAT2_TYPE_HPP
#define MAT2_TYPE_HPP

#include "math/real/type.hpp"
#include "math/vec2/type.hpp"

namespace math {

struct mat2_t {

    MATH_FUNC const vec2_t& operator[](defs::index) const PURE_FUNC;
    MATH_FUNC vec2_t& operator[](defs::index) MUT_FUNC;
    MATH_FUNC float& operator()(defs::index, defs::index) MUT_FUNC;

    static const defs::size size = 2 * vec2_t::size;
    static const defs::size padded_size = 2 * vec2_t::padded_size;

    typedef vec2_t::component_type component_type;
    typedef vec2_t::component_type buffer[size];

private:
    
    union {
        vec2_t columns[2];
        vec2_t::component_type components[padded_size];
    };
};

typedef mat2_t aligned_mat2_t ATTRS(ATTR_ALIGNED(16));

} // namespace math

#endif

