#ifndef MATH_IVEC3_TYPE
#define MATH_IVEC3_TYPE

#include "math/mdefs.hpp"

namespace math {

struct ivec3_t
{
    int32_t components[3];
    inline constexpr int32_t &operator[](size_t);
    inline constexpr int32_t operator[](size_t) const;
};

} // namespace math

#endif
