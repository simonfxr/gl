#ifndef MATH_REAL_HPP
#define MATH_REAL_HPP

#include "math/defs.hpp"

MATH_BEGIN_NAMESPACE

#ifdef MATH_REAL_FLOAT
typedef float real;
#elif defined(MATH_REAL_DOUBLE)
typedef double real;
#endif

MATH_END_NAMESPACE

#endif
