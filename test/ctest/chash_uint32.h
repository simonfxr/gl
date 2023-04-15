#ifndef CHASH_UINT32_H_
#define CHASH_UINT32_H_

#include "chash_prelude.h"

typedef uint32_t ch_Key;
typedef uint32_t ch_Value;

inline ch_Hash
ch_hash(ch_Key x0)
{
    uint64_t x = x0;
    // simplified version of splitmix64:
    // https://xoshiro.di.unimi.it/splitmix64.c
    x ^= x >> 30;
    x *= 0xbf58476d1ce4e5b9U;
    x ^= x >> 27;
    return x;
}

inline bool
ch_equal(ch_Key x, ch_Key y)
{
    return x == y;
}

#endif // CHASH_UINT32_H_
