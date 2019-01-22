#ifndef BL_ENUMERATE_HPP
#define BL_ENUMERATE_HPP

#include "bl/iterator.hpp"

#include <utility>

namespace bl {

template<typename Idx, typename It>
struct Enumerated
{
    Idx _index;
    It _iter;

    constexpr Enumerated &operator++()
    {
        ++_index;
        ++_iter;
        return *this;
    }

    constexpr bool operator==(const Enumerated &rhs) const
    {
        return _iter == rhs._iter;
    }

    constexpr bool operator!=(const Enumerated &rhs) const
    {
        return !operator==(rhs);
    }

    constexpr std::pair<Idx, typename bl::iterator_traits<It>::reference>
    operator*()
    {
        return { _index, *_iter };
    }
};

template<typename Idx, typename It>
struct EnumeratedRange
{
    It _begin;
    It _end;

    constexpr Enumerated<Idx, It> begin() const { return { Idx{}, _begin }; }
    constexpr Enumerated<Idx, It> end() const { return { Idx{}, _end }; }
};

template<typename Idx = size_t, typename T>
inline constexpr auto
enumerate(T &&coll)
{
    using Iter = std::decay_t<decltype(coll.begin())>;
    return EnumeratedRange<Idx, Iter>{ coll.begin(), coll.end() };
}

} // namespace bl

#endif
