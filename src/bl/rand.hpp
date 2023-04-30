#ifndef BL_RAND_HPP
#define BL_RAND_HPP

#include <bl/concepts.hpp>
#include <bl/ptr.hpp>
#include <bl/type_traits.hpp>

#include <bo/cxx.hpp>

#include <algorithm>
#include <array>
#include <bl/span.hpp>

namespace bl::rand {

template<typename T>
concept rng =
  requires(std::decay_t<T> r) {
      typename decltype(r)::result_type;
      requires std::unsigned_integral<typename decltype(r)::result_type>;
      {
          r()
          } -> std::same_as<typename decltype(r)::result_type>;
  };

// *Really* minimal PCG32 code / (c) 2014 M.E. O'Neill / pcg-random.org
// Licensed under Apache License 2.0 (NO WARRANTY, etc. see website)

struct gen32
{
    using state_type = uint64_t;
    using result_type = uint32_t;
    state_type _state = 113ull;
    uint64_t _inc = 1442695040888963407ull;

    constexpr result_type operator()() noexcept
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

static_assert(rng<gen32>);

#define bl_rand_int128_constant(hi, lo)                                        \
    ((static_cast<unsigned __int128>((hi)) << 64) | static_cast<uint64_t>((lo)))

struct gen64
{
    using state_type = unsigned __int128;
    using result_type = uint64_t;
    state_type _state = 113ull;
    unsigned __int128 _inc =
      bl_rand_int128_constant(6364136223846793005ull, 1442695040888963407ull);

    constexpr inline result_type operator()()
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

static_assert(rng<gen64>);

using gen = gen32;

struct BL_API random_device
{
    using result_type = uint64_t;

    random_device() noexcept;
    random_device(const random_device &) noexcept;
    random_device(random_device &&) noexcept;
    random_device &operator=(random_device &&) noexcept;
    random_device &operator=(const random_device &) noexcept;
    ~random_device();

    result_type operator()() noexcept;

    void seed_bytes(bl::span<byte>) noexcept;

private:
    struct Impl;
    owned_ptr<Impl> _p;
};

static_assert(rng<random_device>);

constexpr void
seed_bytes(rng auto &r, bl::span<byte> bytes) noexcept
{
    auto dest = bytes.begin();
    for (auto rest = bytes.size(); rest > 0;) {
        auto w = r();
        auto n = std::min(rest, sizeof(w));
        memcpy(&*dest, &w, n);
        std::advance(dest, n);
        rest -= n;
    }
}

template<rng T = gen>
    requires std::constructible_from<T>
T
seed_from(rng auto &&seeder) noexcept
{
    using state_type = typename T::state_type;
    std::array<byte, sizeof(state_type)> seed;
    seed_bytes(seeder, seed);
    T gen;
    gen.seed(std::bit_cast<state_type>(seed));
    return gen;
}

template<rng T = gen>
    requires std::constructible_from<T>
T
seed_from(std::unsigned_integral auto seedbits) noexcept
{
    using state_type = typename T::state_type;
    std::array<byte, sizeof(state_type)> seed;

    if constexpr (sizeof(state_type) <= sizeof seedbits) {
        auto sbytes =
          std::bit_cast<std::array<byte, sizeof(seedbits)>>(seedbits);
        std::copy_n(sbytes.begin(), sizeof(state_type), seed.begin());
        T gen;
        gen.seed(std::bit_cast<state_type>(seed));
        return gen;
    } else {
        gen32 sd;
        sd.seed(static_cast<gen32::state_type>(seedbits));
        (void) sd();
        return seed_from<T>(sd);
    }
}

struct basic_distribution
{};

inline constexpr struct sample_f
{
    template<typename R, typename... Ts>
        requires invocable<sample_f, R, Ts...>
    BL_inline constexpr auto operator()(R &&r, Ts &&...ts) const
      noexcept(nothrow_invocable<sample_f, R, Ts...>)
        -> bl::invoke_result_t<sample_f, R, Ts...>
    {
        return ::bl::invoke(*this, bl_fwd(r), bl_fwd(ts)...);
    }
} sample;

template<typename D, typename R = gen>
concept a_distribution = rng<R> && invocable<sample_f, D, R>;

template<typename D, rng R = gen>
    requires a_distribution<D, R>
using sampled_t = invoke_result_t<sample_f, D, R>;

template<typename D, typename T, typename R = gen>
concept distribution_over =
  a_distribution<D, R> && std::same_as<sampled_t<D, R>, T>;

struct basic_sampler
{};

inline constexpr struct sampler_f
{
    template<typename R, typename... Ts>
        requires invocable<sampler_f, R, Ts...>
    BL_inline constexpr auto operator()(R &&r, Ts &&...ts) const
      noexcept(nothrow_invocable<sampler_f, R, Ts...>)
        -> bl::invoke_result_t<sampler_f, R, Ts...>
    {
        return ::bl::invoke(*this, bl_fwd(r), bl_fwd(ts)...);
    }
} sampler;

} // namespace bl::rand

#endif // BL_RAND_HPP
