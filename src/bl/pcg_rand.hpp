#ifndef BL_PCG_RAND_HPP
#define BL_PCG_RAND_HPP

#include "bl/core.hpp"

#include <cstdint>

namespace bl {

struct pcg32
{
    uint64_t _state;
    uint64_t _inc;

    constexpr pcg32() : pcg32(1) {}
    constexpr explicit pcg32(uint64_t st)
      : pcg32(st, uint64_t(1442695040888963407ul))
    {}
    constexpr pcg32(uint64_t st, uint64_t inc) : _state(st), _inc(inc) {}

    uint32_t rand() noexcept
    {
        auto oldstate = _state;
        _state = oldstate * 6364136223846793005ULL + (_inc | 1);
        uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
        uint32_t rot = oldstate >> 59u;
        return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
    }
};

} // namespace bl

#endif
