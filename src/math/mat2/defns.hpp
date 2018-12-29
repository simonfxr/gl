#ifndef MAT2_DEFNS_HPP
#define MAT2_DEFNS_HPP

#include "math/mat2/type.hpp"

MATH_BEGIN_NAMESPACE

MATH_FUNC mat2_t
mat2() PURE_FUNC;

MATH_FUNC mat2_t
mat2(real x) PURE_FUNC;

MATH_FUNC mat2_t
mat2(const mat2_t::buffer) PURE_FUNC;

MATH_FUNC mat2_t
mat2(const vec2_t &c1, const vec2_t &c2) PURE_FUNC;

MATH_FUNC void
load(mat2_t::buffer, const mat2_t &) MUT_FUNC;

MATH_END_NAMESPACE

#endif
