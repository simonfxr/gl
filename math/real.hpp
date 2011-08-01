#ifndef MATH_HPP
#define MATH_HPP

#include "math/real/defns.hpp"

#if defined(MATH_INLINE) && !defined(MATH_REAL_INLINE)
#define MATH_REAL_INLINE
#include "math/real/impl.cpp"
#endif

#endif
