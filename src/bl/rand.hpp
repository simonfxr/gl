#ifndef BL_RAND_HPP
#define BL_RAND_HPP

#include <bl/conf.hpp>

#include <array>
#include <concepts>
#include <span>
#include <type_traits>

#include <bo/cxx.hpp>

namespace bl::rand {

template<typename T>
struct sampler
{};

template<typename R>
struct rand_traits
{
    template<std::integral I>
    constexpr I range(I lo, I hi)
    {
        using U = std::make_unsigned_t<I>;
        return lo + static_cast<I>(range(static_cast<U>(hi - lo)));
    }

    template<std::signed_integral I>
    constexpr I range(I hi)
    {
        using U = std::make_unsigned_t<I>;
        return static_cast<I>(range(static_cast<U>(hi)));
    }

    template<std::unsigned_integral U>
    constexpr U range(U hi)
    {
        using UU = bo::widen_int_t<U>;
        // https://lemire.me/blog/2016/06/30/fast-random-shuffling/
        auto r = sample<U>();
        auto multiresult = static_cast<UU>(r) * static_cast<UU>(hi);
        auto leftover = static_cast<U>(multiresult);
        if (leftover < hi) {
            auto threshold = -hi % hi;
            while (leftover < threshold) {
                r = sample<U>();
                multiresult = static_cast<UU>(r) * static_cast<UU>(hi);
                leftover = static_cast<U>(multiresult);
            }
        }
        return static_cast<U>(multiresult >> (8 * sizeof(U)));
    }

    [[nodiscard]] constexpr auto operator()() noexcept
    {
        return self().generate();
    }

    template<typename T>
    constexpr T sample();

private:
    constexpr R &self() noexcept { return static_cast<R &>(*this); }
};

// *Really* minimal PCG32 code / (c) 2014 M.E. O'Neill / pcg-random.org
// Licensed under Apache License 2.0 (NO WARRANTY, etc. see website)

struct gen32 : rand_traits<gen32>
{
    using state_type = uint64_t;
    using result_type = uint32_t;
    state_type _state = 113ull;
    uint64_t _inc = 1442695040888963407ull;

    constexpr inline result_type generate()
    {
        auto oldstate = _state;
        _state = oldstate * 6364136223846793005ull + (_inc | 1);
        auto xorshifted =
          static_cast<uint32_t>(((oldstate >> 18u) ^ oldstate) >> 27u);
        auto rot = static_cast<uint32_t>(oldstate >> 59u);
        return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
    }

    constexpr void seed(const state_type &seed) { _state = seed; }
};

#define bl_rand_int128_constant(hi, lo)                                        \
    ((static_cast<unsigned __int128>((hi)) << 64) | static_cast<uint64_t>((lo)))

struct gen64 : rand_traits<gen64>
{
    using state_type = unsigned __int128;
    using result_type = uint64_t;
    state_type _state = 113ull;
    unsigned __int128 _inc =
      bl_rand_int128_constant(6364136223846793005ull, 1442695040888963407ull);

    constexpr inline result_type generate()
    {
        auto oldstate = _state;
        _state = oldstate * bl_rand_int128_constant(2549297995355413924ull,
                                                    4865540595714422341ull) +
                 (_inc | 1);
        auto value =
          static_cast<uint64_t>(_state >> 64u) ^ static_cast<uint64_t>(_state);
        auto rot = static_cast<unsigned>(_state >> 122u);
        return (value >> rot) | (value << ((-rot) & 63));
    }

    constexpr void seed(const state_type &seed) { _state = seed; }
};

using gen = gen64;

BL_API void seed_bytes(std::span<std::byte>) noexcept;

template<typename T>
constexpr void
seed_bytes(T &rng, std::span<std::byte> bytes) noexcept
{
    auto dest = bytes.begin();
    for (auto rest = bytes.size(); rest > 0;) {
        auto w = rng();
        auto n = std::min(rest, sizeof(w));
        memcpy(&*dest, &w, n);
        std::advance(dest, n);
        rest -= n;
    }
}

template<typename T = gen>
T
seed() noexcept
{
    using state_type = typename T::state_type;
    std::array<std::byte, sizeof(state_type)> seed;
    seed_bytes(seed);
    T gen;
    gen.seed(std::bit_cast<state_type>(seed));
    return gen;
}

template<typename T = gen>
T
seed(uint32_t seed32) noexcept
{
    gen32 sd;
    sd.seed(seed32);
    (void) sd();

    using state_type = typename T::state_type;
    std::array<std::byte, sizeof(state_type)> seed;
    seed_bytes(sd, seed);
    T gen;
    gen.seed(std::bit_cast<state_type>(seed));
    return gen;
}

template<typename T, typename R>
T
sample(R &rng)
{
    return sample(rng, sampler<T>{});
}

template<std::floating_point T>
constexpr T
sample(auto &rng, sampler<T>)
{
    auto x = rng();
    return static_cast<T>(x) *
           (T{ 1 } / static_cast<T>(static_cast<decltype(x)>(-1)));
}

template<std::unsigned_integral T>
constexpr T
sample(auto &rng, sampler<T>)
{
    using O = decltype(rng());
    if constexpr (sizeof(T) <= sizeof(O)) {
        return static_cast<T>(rng());
    } else {
        using U = bo::narrow_int_t<T>;
        auto hi = sample<U>(rng);
        auto lo = sample<U>(rng);
        return (static_cast<T>(hi) << (8 * sizeof(U))) | static_cast<T>(lo);
    }
}

template<typename R>
template<typename T>
constexpr T
rand_traits<R>::sample()
{
    using bl::rand::sample;
    return sample(self(), sampler<T>{});
}

} // namespace bl::rand

#endif // BL_RAND_HPP
