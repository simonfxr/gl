#ifndef MATH_DEFS_HPP
#define MATH_DEFS_HPP

#ifndef ATTR_CONST
#define ATTR_CONST
#endif

#ifdef MATH_INLINE

#define PURE_FUNC __attribute__((ATTR_CONST always_inline, unused, nothrow))
#define MUT_FUNC __attribute__((always_inline, unused, nothrow))
#define MATH_BEGIN_NAMESPACE namespace math { namespace {
#define MATH_END_NAMESPACE } }
#define MATH_INLINE_SPEC inline
#define MATH_CONSTANT __attribute__((unused))

#else

#define PURE_FUNC __attribute__((ATTR_CONST nothrow))
#define MUT_FUNC __attribute__((nothrow))
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
