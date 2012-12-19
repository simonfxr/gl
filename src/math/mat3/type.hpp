#ifndef MAT3_TYPE_HPP
#define MAT3_TYPE_HPP

#include "math/real/type.hpp"
#include "math/vec3/type.hpp"

namespace math {

struct mat3_t {

    MATH_FUNC const vec3_t& operator[](defs::index) const PURE_FUNC;
    MATH_FUNC vec3_t& operator[](defs::index) MUT_FUNC;
    MATH_FUNC float& operator()(defs::index, defs::index) MUT_FUNC;

    static const defs::size size = 3 * vec3_t::size;
    static const defs::size padded_size = 3 * vec3_t::padded_size;

    typedef vec3_t::component_type component_type;
    typedef vec3_t::component_type buffer[size];

private:
    
    union {
        vec3_t columns[3];
        vec3_t::component_type components[padded_size];
    };
};

typedef mat3_t aligned_mat3_t ATTRS(ATTR_ALIGNED(16));

} // namespace math

#endif

