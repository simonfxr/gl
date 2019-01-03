#ifndef MATH_IVEC3_TYPE
#define MATH_IVEC3_TYPE

#include "math/mdefs.hpp"

namespace math {

struct ivec3_t
{
    int32_t components[3];
    inline constexpr int32_t &operator[](size_t i) { return components[i]; }

    inline constexpr int32_t operator[](size_t i) const
    {
        return components[i];
    }
};

} // namespace math

#endif
