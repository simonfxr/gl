#ifndef _DEFS_H
#define _DEFS_H

typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned int   uint32;
typedef unsigned long  uint64;

typedef float  real32;
typedef double real64;

#define likely(e) __builtin_expect((e) != 0, 1)
#define unlikely(e) __builtin_expect((e) != 0, 0)

#endif
