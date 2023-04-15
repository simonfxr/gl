#ifndef CRAND_H_
#define CRAND_H_

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

// The following code can be found here:
// https://www.pcg-random.org/download.html
// It has been slightly modifed.
// The snippet extends until END PCG32
// BEGIN PCG32

// *Really* minimal PCG32 code / (c) 2014 M.E. O'Neill / pcg-random.org
// Licensed under Apache License 2.0 (NO WARRANTY, etc. see website)

typedef struct crand_rng
{
    uint64_t state;
    uint64_t inc;
} crand_rng;

inline uint32_t
crand_random_r(crand_rng *rng)
{
    uint64_t oldstate = rng->state;
    // Advance internal state
    rng->state = oldstate * 6364136223846793005ULL + (rng->inc | 1);
    // Calculate output function (XSH RR), uses old state for max ILP
    uint32_t xorshifted = (uint32_t) (((oldstate >> 18u) ^ oldstate) >> 27u);
    uint32_t rot = (uint32_t) (oldstate >> 59u);
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

// END PCG32

#define crand_int(type, rng) ((type) crand_bits((rng), 8 * sizeof(type)))

inline crand_rng
crand_derive(uint64_t seed, uint64_t tweak)
{
    crand_rng rng = { seed, 0x10070D7AEBFB045Bull };
    crand_random_r(&rng);
    rng.state ^= tweak;
    return rng;
}

// FNV-1a hash with custom salt tweak, see:
// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
inline uint64_t
crand_strhash(uint64_t h0, const char *s)
{
    uint64_t h = 0xCBF29CE484222325ull;
    for (; *s; h *= 0x100000001B3ull)
        h ^= (uint8_t) *s++;
    if (h0)
        (void) (h ^= h0), h *= 0x567190B2E1A3A451ull;
    return h;
}

inline crand_rng
crand_derive_str(uint64_t seed, const char *s)
{
    return crand_derive(seed, crand_strhash(0x1, s));
}

inline uint64_t
crand_bits(crand_rng *rng, unsigned nbits)
{
    uint64_t x = crand_random_r(rng);
    x <<= 32;
    if (nbits > 32)
        x |= crand_random_r(rng);
    x >>= 64 - nbits;
    return x;
}

inline char *
crand_bytes(crand_rng *rng, char *buf, size_t n)
{
    size_t k;
    for (; n; n -= k) {
        uint32_t x = crand_random_r(rng);
        k = (n >= sizeof(x) ? sizeof(x) : n);
        memcpy(buf, &x, k);
        buf += sizeof(x);
    }
    return buf;
}

void
crand_seed(crand_rng *rng);
#ifdef __cplusplus
}
#endif
#endif // CRAND_H_

#ifdef CRAND_IMPLEMENTATION
#ifdef __cplusplus
extern "C" {
#endif

void
crand_seed(crand_rng *rng)
{
    uintptr_t v[1] = { 0 };
    uint64_t x0 = (uintptr_t) (void *) v;

#ifdef __GNUC__
    __asm__("" : "+r"(x0));
#endif

    uint64_t x[] = {
        x0,
        (uintptr_t) (void *) &memcpy,
        (uintptr_t) (void *) &crand_seed,
    };

    rng->state = (uint64_t) -1ll;
    for (size_t i = 0; i < sizeof(x) / sizeof(x[0]); ++i) {
        rng->state ^= x[i];
        crand_random_r(rng);
    }
}

#ifdef __cplusplus
}
#endif
#endif // CRAND_IMPLEMENTATION
