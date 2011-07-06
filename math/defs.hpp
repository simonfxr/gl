#ifndef MATH_DEFS_HPP
#define MATH_DEFS_HPP

#define MATH_SSE_VERS 401

#include "defs.h"

#ifdef MATH_INLINE

#define PURE_FUNC ATTRS(ATTR_FORCE_INLINE, ATTR_NO_WARN_UNUSED_DEF, ATTR_NOTHROW)
#define MUT_FUNC ATTRS(ATTR_FORCE_INLINE, ATTR_NO_WARN_UNUSED_DEF, ATTR_NOTHROW)
#define MATH_BEGIN_NAMESPACE namespace math { namespace {
#define MATH_END_NAMESPACE } }
#define MATH_INLINE_SPEC inline
#define MATH_CONSTANT ATTRS(ATTR_NO_WARN_UNUSED_DEF)

#else

#define PURE_FUNC ATTRS(ATTR_NOTHROW)
#define MUT_FUNC ATTRS(ATTR_NOTHROW)
#define MATH_BEGIN_NAMESPACE namespace math {
#define MATH_END_NAMESPACE }
#define MATH_INLINE_SPEC
#define MATH_CONSTANT

#endif

#ifndef MATH_SSE_VERS
#define MATH_SSE_VERS 0
#endif

#define MATH_MK_SSE_VERS(maj, min) ((maj) * 100 + (min))

#define MATH_SSE(maj, min) (MATH_SSE_VERS >= MATH_MK_SSE_VERS(maj, min))

#endif
