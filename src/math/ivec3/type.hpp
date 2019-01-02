#ifndef MATH_IVEC3_TYPE
#define MATH_IVEC3_TYPE

#include "math/mdefs.hpp"

namespace math {

struct ivec3_t
{
    defs::int32_t components[3];
    constexpr MATH_FUNC defs::int32_t &operator[](defs::index_t) MUT_FUNC;
    constexpr MATH_FUNC defs::int32_t operator[](defs::index_t) const PURE_FUNC;
};

} // namespace math

#endif
