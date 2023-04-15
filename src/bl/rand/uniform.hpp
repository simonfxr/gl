#ifndef BL_RAND_UNIFORM_HPP
#define BL_RAND_UNIFORM_HPP

#include <bl/invoke.hpp>
#include <bl/rand.hpp>

#include <bo/cxx.hpp>

namespace bl::rand {

template<typename T>
    requires requires(const std::array<std::byte, sizeof(T)> &bytes) {
                 std::bit_cast<T>(bytes);
             }
struct uniform_bitrep
{
    friend T invoke(sample_f, const uniform_bitrep<T> &, rng auto &&r)
        requires(std::signed_integral<T>)
    {
        std::array<std::byte, sizeof(T)> bits;
        seed_bytes(r, bits);
        return std::bit_cast<T>(bits);
    }
};

template<typename T>
struct uniform_distribution
{
    T lo;
    T l;

    explicit uniform_distribution()
        requires std::floating_point<T>
      : lo(static_cast<T>(0)), l(static_cast<T>(1))
    {}

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
                 uniform_distribution{ static_cast<U>(d.l) }, bl_fwd(g)));
    }

    friend T invoke(sample_f, const uniform_distribution<T> &d, rng auto &&g)
        requires(std::unsigned_integral<T>)
    {
        auto &l = d.l;
        using UU = bo::widen_int_t<T>;
        // https://lemire.me/blog/2016/06/30/fast-random-shuffling/
        auto r = bl::rand::sample(uniform_bitrep<T>{}, g);
        auto multiresult = static_cast<UU>(r) * static_cast<UU>(l);
        auto leftover = static_cast<T>(multiresult);
        if (leftover < l) {
            auto threshold = -l % l;
            while (leftover < threshold) {
                r = bl::rand::sample(uniform_bitrep<T>{}, g);
                multiresult = static_cast<UU>(r) * static_cast<UU>(l);
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

} // namespace bl::rand

#endif // BL_RAND_UNIFORM_HPP
