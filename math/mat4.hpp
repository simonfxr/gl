#ifndef MAT4_HPP
#define MAT4_HPP

#include "math/mat4/type.hpp"
#include "math/mat4/defns.hpp"

#if defined(MATH_INLINE) && !defined(MATH_MAT4_INLINE)
#define MATH_MAT4_INLINE
#include "math/mat4/impl.cpp"
#endif

#endif
