#ifndef VEC3_HPP
#define VEC3_HPP

#include "math/vec3/type.hpp"
#include "math/vec3/defns.hpp"

#if defined(MATH_INLINE) && !defined(MATH_VEC3_INLINE)
#define MATH_VEC3_INLINE
#include "math/vec3/impl.cpp"
#endif

#endif
