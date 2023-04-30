#ifndef BL_RAND_UNIFORM_HPP
#define BL_RAND_UNIFORM_HPP

#include <bl/invoke.hpp>
#include <bl/mp.hpp>
#include <bl/rand.hpp>

#include <bo/cxx.hpp>

namespace bl::rand {

struct basic_uniform_distribution : basic_distribution
{};

template<bitcastable T>
struct uniform_bitrep : basic_uniform_distribution
{
    friend T invoke(sample_f, const uniform_bitrep<T> &, rng auto &&r)
    {
        std::array<byte, sizeof(T)> bits{};
        seed_bytes(r, bits);
        return std::bit_cast<T>(bits);
    }
};

template<typename T>
concept a_uniform_scalar =
  std::integral<std::decay_t<T>> || std::floating_point<std::decay_t<T>>;

template<a_uniform_scalar T>
struct uniform_distribution : basic_uniform_distribution
{
    T lo;
    T l;

    explicit uniform_distribution()
        requires std::floating_point<T>
      : lo(static_cast<T>(0)), l(static_cast<T>(1))
    {}

    uniform_distribution()
        requires(!std::floating_point<T>)
    = delete;

    explicit uniform_distribution(T &&l) : lo(static_cast<T>(0)), l(bl_fwd(l))
    {}
    uniform_distribution(T &&lo, T &&hi)
      : lo(bl_fwd(lo)), l(bl_fwd(hi) - this->lo)
    {}

    friend T invoke(sample_f, const uniform_distribution<T> &d, rng auto &&g)
        requires(std::signed_integral<T>)
    {
        using U = std::make_unsigned_t<T>;
        return d.lo +
               static_cast<T>(bl::rand::sample(
                 uniform_distribution<U>{ static_cast<U>(d.l) }, bl_fwd(g)));
    }

    friend T invoke(sample_f, const uniform_distribution<T> &d, rng auto &&g)
        requires(std::unsigned_integral<T>)
    {
        auto &dl = d.l;
        using UU = bo::widen_int_t<T>;
        // https://lemire.me/blog/2016/06/30/fast-random-shuffling/
        auto r = bl::rand::sample(uniform_bitrep<T>{}, g);
        auto multiresult = static_cast<UU>(r) * static_cast<UU>(dl);
        auto leftover = static_cast<T>(multiresult);
        if (leftover < dl) {
            auto threshold = -dl % dl;
            while (leftover < threshold) {
                r = bl::rand::sample(uniform_bitrep<T>{}, g);
                multiresult = static_cast<UU>(r) * static_cast<UU>(dl);
                leftover = static_cast<T>(multiresult);
            }
        }
        return d.lo + static_cast<T>(multiresult >> (8 * sizeof(T)));
    }

    friend T invoke(sample_f, const uniform_distribution<T> &d, rng auto &&g)
        requires(std::floating_point<T>)
    {
        auto k = g();
        auto x = k * T{ 1 } / static_cast<T>(static_cast<decltype(k)>(-1));
        return d.lo + d.l * static_cast<T>(x);
    }
};

template<typename T, typename... Args>
uniform_distribution(T, Args...) -> uniform_distribution<T>;

struct uniform_sampler : basic_sampler
{
    template<bitcastable T>
        requires std::integral<T>
    friend auto invoke(sampler_f, const uniform_sampler &, type_proxy<T>)
    {
        return uniform_bitrep<T>{};
    }

    template<a_uniform_scalar T>
    friend auto invoke(sampler_f,
                       const uniform_sampler &,
                       type_proxy<T>,
                       std::common_reference_with<T> auto &&lo,
                       std::common_reference_with<T> auto &&...rest)
    {
        return uniform_distribution<std::decay_t<decltype(lo)>>{
            bl_fwd(lo), bl_fwd(rest)...
        };
    }

    template<std::floating_point T>
    friend auto invoke(sampler_f, const uniform_sampler &, type_proxy<T>)
    {
        return uniform_distribution<T>{};
    }
};

template<typename T, typename... Args>
BL_inline auto
uniform(Args &&...args)
{
    return sampler(uniform_sampler{}, type_proxy<T>{}, bl_fwd(args)...);
}

template<typename T, typename... Args>
BL_inline auto
sample_uniform(rng auto &&r, Args &&...args)
{
    return sample(sampler(uniform_sampler{}, type_proxy<T>{}, bl_fwd(args)...),
                  r);
}

} // namespace bl::rand

#endif // BL_RAND_UNIFORM_HPP
