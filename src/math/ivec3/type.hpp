#ifndef MATH_IVEC3_TYPE
#define MATH_IVEC3_TYPE

#include "math/mdefs.hpp"

namespace math {

struct ivec3_t
{
    defs::int32 components[3];

    MATH_FUNC defs::int32 &operator[](defs::index) MUT_FUNC;
    MATH_FUNC defs::int32 operator[](defs::index) const PURE_FUNC;
};

} // namespace math

#endif
