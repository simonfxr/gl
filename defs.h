#ifndef _DEFS_H
#define _DEFS_H

#include "config.h"
#include <stdint.h>

#ifdef SYSTEM_LINUX
#define SYSTEM_UNIX
#endif

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

#define ARRAY_LENGTH(x) (sizeof (x) / sizeof *(x))

#define UNUSED(x) ((void) (x))

#ifdef DEBUG
#define ON_DEBUG(x) do { (x); } while (0)
#define DEBUG_ERR(msg) ERR(msg)
#else
#define ON_DEBUG(x) ((void) 0)
#define DEBUG_ERR(x) ((void) 0)
#endif

#ifdef DEBUG
#define DEBUG_ASSERT(x, err) do { \
        if (unlikely(!(x)))       \
            DEBUG_ERR(err);           \
    } while (0)
#else
#define DEBUG_ASSERT(x, err) ((void) 0)
#endif

#define ASSERT_MSG DEBUG_ASSERT
#define ASSERT(x) ASSERT_MSG(x, "assertion failed: " #x)

#define ERR(e) glt::error(e, __FILE__, __LINE__, __func__)

#define ERROR_ONCE(e) do {                                              \
        static bool __reported = false;                                 \
        if (unlikely(!__reported)) {                                    \
            __reported = true;                                          \
            glt::error_once(e, __FILE__, __LINE__, __func__);           \
        }                                                               \
    } while (0)



#define FATAL_ERR(e) glt::fatal_error(e, __FILE__, __LINE__, __func__)

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

#endif

#define RESTRICT __restrict__

#define LOCAL ATTRS(ATTR_NO_WARN_UNUSED_DEF)

#define CONCAT(a, b) CONCAT_AUX1(a, b)
#define CONCAT_AUX1(a, b) CONCAT_AUX2(a, b)
#define CONCAT_AUX2(a, b) a##b

#define CONCAT3(a, b, c) CONCAT(a, CONCAT(b, c))

#define IALIAS(t) typedef CONCAT(t, _t) t;

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

#endif
