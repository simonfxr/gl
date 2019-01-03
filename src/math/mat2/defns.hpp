#ifndef MAT2_DEFNS_HPP
#define MAT2_DEFNS_HPP

#include "math/mat2/type.hpp"

namespace math {

constexpr inline mat2_t
mat2();

constexpr inline mat2_t
mat2(real x);

constexpr inline mat2_t
mat2(const mat2_t::buffer);

constexpr inline mat2_t
mat2(const vec2_t &c1, const vec2_t &c2);

inline void
load(mat2_t::buffer, const mat2_t &);

} // namespace math

#endif
