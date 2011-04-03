#ifndef _DEFS_H
#define _DEFS_H

typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned int   uint32;
typedef unsigned long  uint64;

typedef uint8 byte;

typedef char  int8;
typedef short int16;
typedef int   int32;
typedef long  int64;

#if defined(linux) || defined(__linux)
#define SYSTEM_LINUX
#define SYSTEM_UNIX
#endif

#ifdef __GXX_EXPERIMENTAL_CXX0X__
#define CXX0X
#endif

#ifdef CXX0X
#define EXPLICIT explicit
#define OVERRIDE override
#else
#define EXPLICIT
#define OVERRIDE
#endif

#define likely(e) __builtin_expect((e) != 0, 1)
#define unlikely(e) __builtin_expect((e) != 0, 0)

#define ARRAY_LENGTH(x) (sizeof (x) / sizeof *(x))

#define UNUSED(x) ((void) (x))

#ifdef DEBUG
#define ON_DEBUG(x) do { (x); } while (0)
#define DEBUG_ERROR(msg) ERROR(msg)
#else
#define ON_DEBUG(x) ((void) 0)
#define DEBUG_ERROR(x) ((void) 0)
#endif

#ifdef DEBUG
#define DEBUG_ASSERT(x, err) do { \
        if (unlikely(!(x)))       \
            DEBUG_ERROR(err);           \
    } while (0)
#else
#define DEBUG_ASSERT(x, err) ((void) 0)
#endif

#define ASSERT_MSG DEBUG_ASSERT
#define ASSERT(x) ASSERT_MSG(x, "assertion failed: " #x)

#define ERROR(e) glt::error(e, __FILE__, __LINE__, __func__)

#define FATAL_ERROR(e) glt::fatal_error(e, __FILE__, __LINE__, __func__)

#define ATTRS(...) __attribute__((__VA_ARGS__))

#define ATTR_WARN_UNUSED warn_unused_result

#define ATTR_NO_WARN_UNUSED_DEF unused

#define ATTR_ALIGNED(n) aligned(n)

#define RESTRICT __restrict__

#define LOCAL ATTRS(ATTR_NO_WARN_UNUSED_DEF)

#define CONCAT(a, b) CONCAT_AUX1(a, b)
#define CONCAT_AUX1(a, b) CONCAT_AUX2(a, b)
#define CONCAT_AUX2(a, b) a##b

#define CONCAT3(a, b, c) CONCAT(a, CONCAT(b, c))

#endif
