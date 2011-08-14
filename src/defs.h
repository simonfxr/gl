#ifndef _DEFS_H
#define _DEFS_H

#include <stdint.h>

#ifdef CXX0X
#define EXPLICIT explicit
#define OVERRIDE override
#else
#define EXPLICIT
#define OVERRIDE
#endif

#ifdef GNU_EXTENSIONS
#define likely(e) __builtin_expect((e) != 0, 1)
#define unlikely(e) __builtin_expect((e) != 0, 0)
#else
#define likely(e) (e)
#define unlikely(e) (e)
#endif

#ifdef GNU_EXTENSIONS
#define HAVE_ALIGNOF_EXPR
#define HAVE_ALIGNOF_TYPE
#define ALIGNOF_EXPR(e) __alignof__(e)
#define ALIGNOF_TYPE(t) __alignof__(t)
#else

#define HAVE_ALIGNOF_TYPE
#define ALIGNOF_TYPE(t) offsetof(struct { char ___c; t ___x; }, ___x)
#define ALIGNOF_EXPR(e) alignof_not_defined

#endif

#define ARRAY_LENGTH(x) (sizeof (x) / sizeof *(x))

#define UNUSED(x) ((void) (x))

#ifdef DEBUG
#define ON_DEBUG(x) do { (x); } while (0)
#define DEBUG_DECL(x) x
#else
#define ON_DEBUG(x) ((void) 0)
#define DEBUG_DECL(x)
#endif

#ifdef GNU_EXTENSIONS
#define ATTRS(...) __attribute__((__VA_ARGS__))
#else
#define ATTRS(...)
#endif

#ifdef GNU_EXTENSIONS

#define ATTR_WARN_UNUSED warn_unused_result
#define ATTR_NO_WARN_UNUSED_DEF unused
#define ATTR_ALIGNED(n) aligned(n)
#define ATTR_NOINLINE noinline
#define ATTR_NOTHROW nothrow
#define ATTR_FORCE_INLINE always_inline
#define ATTR_NORETURN noreturn
#define ATTR_PACKED packed

#endif

#ifdef GNU_EXTENSIONS
#define HAVE_THREAD_LOCAL
#define THREAD_LOCAL(type, var) __thread type var
#else
#define THREAD_LOCAL(type, var) type var
#endif

#define RESTRICT __restrict__

#define LOCAL ATTRS(ATTR_NO_WARN_UNUSED_DEF)
#define LOCAL_CONSTANT ATTRS(ATTR_NO_WARN_UNUSED_DEF)

#define CONCAT(a, b) CONCAT_AUX1(a, b)
#define CONCAT_AUX1(a, b) CONCAT_AUX2(a, b)
#define CONCAT_AUX2(a, b) CONCAT_AUX3(a, b)
#define CONCAT_AUX3(a, b) CONCAT_AUX4(a, b)
#define CONCAT_AUX4(a, b) CONCAT_AUX5(a, b)
#define CONCAT_AUX5(a, b) a##b

#define CONCAT3(a, b, c) CONCAT(a, CONCAT(b, c))

#define AS_STR(a) AS_STRING(a)
#define AS_STRING(a) AS_STRING_AUX1(a)
#define AS_STRING_AUX1(a) AS_STRING_AUX2(a)
#define AS_STRING_AUX2(a) AS_STRING_AUX3(a)
#define AS_STRING_AUX3(a) AS_STRING_AUX4(a)
#define AS_STRING_AUX4(a) AS_STRING_AUX5(a)
#define AS_STRING_AUX5(a) AS_STRING_AUX6(a)
#define AS_STRING_AUX6(a) AS_STRING_AUX7(a)
#define AS_STRING_AUX7(a) #a

#define IALIAS(t) typedef t##_t t;

IALIAS(uint8);
IALIAS(uint16);
IALIAS(uint32);
IALIAS(uint64);

IALIAS(int8);
IALIAS(int16);
IALIAS(int32);
IALIAS(int64);

#undef IALIAS

typedef uintptr_t uptr;
typedef intptr_t iptr;

typedef uint8 byte;

typedef uptr index_t;
typedef iptr sindex_t;

#endif
