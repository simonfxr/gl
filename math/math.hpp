#ifndef MATH_HPP
#define MATH_HPP

#include "math/math/defns.hpp"

#if defined(MATH_INLINE) && !defined(MATH_MATH_INLINE)
#define MATH_MATH_INLINE
#include "math/math/impl.cpp"
#endif

#endif
