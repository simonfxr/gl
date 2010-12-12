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

#define likely(e) __builtin_expect((e) != 0, 1)
#define unlikely(e) __builtin_expect((e) != 0, 0)

#define ARRAY_LENGTH(x) (sizeof (x) / sizeof *(x))

#define UNUSED(x) ((void) (x))

#ifdef DEBUG
#define ON_DEBUG(x) do { (x); } while (0)
#else
#define ON_DEBUG(x) ((void) 0)
#endif

#ifdef DEBUG
#define DEBUG_ASSERT(x, err) do { \
        if (unlikely(!(x)))       \
            ERROR(err);           \
    } while (0)
#else
#define DEBUG_ASSERT(x, err) ((void) 0)
#endif

#define ASSERT DEBUG_ASSERT

#define ERROR(e) gltools::error(e, __FILE__, __LINE__, __func__)

#endif
