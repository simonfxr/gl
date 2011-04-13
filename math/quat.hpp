#ifndef MATH_QUAT_HPP
#define MATH_QUAT_HPP

#include "math/quat/type.hpp"
#include "math/quat/defns.hpp"

#if defined(MATH_INLINE) && !defined(MATH_QUAT_INLINE)
#define MATH_QUAT_INLINE
#include "math/quat/impl.cpp"
#endif

#endif
