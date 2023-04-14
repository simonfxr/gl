#ifndef MATH_VEC2_HPP
#define MATH_VEC2_HPP

#include "math/genvec.hpp"

namespace math {

using vec2 = genvec<real, 2>;
using vec2_t = vec2;

inline vec2
cossin(real rad)
{
    return { cos(rad), sin(rad) };
}

} // namespace math

#endif
