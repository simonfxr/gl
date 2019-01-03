#ifndef MATH_DEFS_HPP
#define MATH_DEFS_HPP

#include "defs.hpp"

#if defined(MATH_REAL_FLOAT) && defined(MATH_REAL_DOUBLE)
#error "MATH_REAL_FLOAT and MATH_REAL_DOUBLE both defined"
#elif !defined(MATH_REAL_FLOAT) && !defined(MATH_REAL_DOUBLE)
#define MATH_REAL_FLOAT 1
#endif

#endif
