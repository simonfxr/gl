#ifndef MATH_DEFS_HPP
#define MATH_DEFS_HPP

#define CONST_ATTR

#ifdef MATH_INLINE

#define PURE_FUNC __attribute__((CONST_ATTR always_inline, unused, nothrow))
#define MUT_FUNC __attribute__((always_inline, unused, nothrow))
#define MATH_BEGIN_NAMESPACE namespace math { namespace {
#define MATH_END_NAMESPACE } }
#define MATH_INLINE_SPEC inline
#define MATH_CONSTANT __attribute__((unused))

#else

#define PURE_FUNC __attribute__((CONST_ATTR nothrow))
#define MUT_FUNC __attribute__((nothrow))
#define MATH_BEGIN_NAMESPACE namespace math {
#define MATH_END_NAMESPACE }
#define MATH_INLINE_SPEC
#define MATH_CONSTANT

#endif

#ifndef MATH_SSE_VERS
#define MATH_SSE_VERS 0
#endif

#define MATH_SSE(maj, min) (MATH_SSE_VERS >= ((maj) * 10 + min))

#endif
