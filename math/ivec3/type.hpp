#ifndef MATH_IVEC3_TYPE
#define MATH_IVEC3_TYPE

#include "math/defs.hpp"

namespace math {

struct ivec3_t {
    int32 components[3];

    int32& operator[](index_t) MUT_FUNC;
    int32 operator[](index_t) const PURE_FUNC;
};

} // namespace math

#endif
