#ifndef MATH_REAL_HPP
#define MATH_REAL_HPP

#include "math/mdefs.hpp"

namespace math {

#ifdef MATH_REAL_FLOAT
typedef float real;
#define R_FMT "f"
#elif defined(MATH_REAL_DOUBLE)
typedef double real;
#define R_FMT "lf"
#endif

} // namespace math

#endif
