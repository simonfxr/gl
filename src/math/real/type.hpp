#ifndef MATH_REAL_HPP
#define MATH_REAL_HPP

#include "math/mdefs.hpp"

namespace math {

#ifdef MATH_REAL_FLOAT
typedef float real;
#elif defined(MATH_REAL_DOUBLE)
typedef double real;
#endif

} // namespace math

#endif
