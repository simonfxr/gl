#ifndef MATH_IVEC2_HPP
#define MATH_IVEC2_HPP

#include "math/genvec.hpp"

namespace math {

using ivec2_t = genvec<int32_t, 2>;

inline constexpr ivec2_t
ivec2(int32_t x, int32_t y)
{
    return ivec2_t::make(x, y);
}

template<typename T>
inline constexpr ivec2_t
ivec2(const genvec<T, 2> &v)
{
    return ivec2_t::convert(v);
}

inline constexpr ivec2_t
ivec2(int32_t x)
{
    return ivec2_t::broadcast(x);
}

template<typename T>
inline constexpr ivec2_t
ivec2(const genvec<T, 3> &v)
{
    return ivec2(v[0], v[1]);
}
} // namespace math

#endif
