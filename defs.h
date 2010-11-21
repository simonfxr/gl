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

#endif
