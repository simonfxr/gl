#ifndef MATH_DEFS_HPP
#define MATH_DEFS_HPP

#define CONST_ATTR

#ifdef MATH_INLINE

#define PURE_FUNC __attribute__((CONST_ATTR always_inline, unused, nothrow))
#define MUT_FUNC __attribute__((always_inline, unused, nothrow))
#define MATH_BEGIN_NAMESPACE namespace math { namespace {
#define MATH_END_NAMESPACE } }

#else

#define PURE_FUNC __attribute__((CONST_ATTR nothrow))
#define MUT_FUNC __attribute__((nothrow))
#define MATH_BEGIN_NAMESPACE namespace math {
#define MATH_END_NAMESPACE }

#endif

#endif
