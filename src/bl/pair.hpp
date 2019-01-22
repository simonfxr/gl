#ifndef BL_PAIR_HPP
#define BL_PAIR_HPP

#include "bl/core.hpp"

namespace bl {

template<typename A, typename B>
struct pair
{
    HU_FORCE_INLINE constexpr const A &fst() const { return _fst; }
    HU_FORCE_INLINE constexpr A &fst() { return _fst; }

    HU_FORCE_INLINE constexpr const B &snd() const { return _snd; }
    HU_FORCE_INLINE constexpr B &snd() { return _snd; }

    template<typename U, typename V>
    constexpr pair(U &&x, V &&y)
      : _fst(std::forward<U>(x)), _snd(std::forward<V>(y))
    {}

private:
    A _fst;
    B _snd;
};

template<typename A, typename B>
HU_FORCE_INLINE inline constexpr pair<A, B>
make_pair(A fst, B snd)
{
    return pair<A, B>(move(fst), move(snd));
}

} // namespace bl

#endif
