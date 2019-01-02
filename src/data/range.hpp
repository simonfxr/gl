#ifndef DATA_RANGE_HPP
#define DATA_RANGE_HPP

#include "defs.hpp"

#include <iterator>

template<typename T>
struct IntIterator
{
    T _index;
    T _step;

    using difference_type = ptrdiff_t;
    using value_type = const T;
    using pointer = std::add_pointer_t<const T>;
    using reference = const T &;
    using iterator_category = std::random_access_iterator_tag;

    constexpr IntIterator &operator++()
    {
        _index += _step;
        return *this;
    }

    constexpr IntIterator operator++(int)
    {
        auto cur = *this;
        ++*this;
        return cur;
    }

    constexpr IntIterator &operator--()
    {
        _index -= _step;
        return *this;
    }

    constexpr IntIterator operator--(int)
    {
        auto cur = *this;
        --*this;
        return cur;
    }

    constexpr difference_type operator-(const IntIterator &rhs) const
    {
        return (_index - rhs._index) / _step;
    }

    constexpr IntIterator operator-(difference_type t) const
    {
        return { _index - _step * t, _step };
    }

    constexpr IntIterator operator+(difference_type t) const
    {
        return { _index + _step * t, _step };
    }

    constexpr IntIterator &operator+=(difference_type t)
    {
        return *this = *this + t;
    }

    constexpr IntIterator &operator-=(difference_type t)
    {
        return *this = *this - t;
    }

    constexpr reference operator*() const { return _index; }
    constexpr pointer operator->() const { return &_index; }

    constexpr bool operator==(const IntIterator &rhs) const
    {
        return _index == rhs._index;
    }

    constexpr bool operator!=(const IntIterator &rhs) const
    {
        return !operator==(rhs);
    }
};

template<typename T>
struct IntRange
{
    T _start;
    T _step;
    T _lim;

    constexpr IntIterator<T> begin() const { return { _start, _step }; }
    constexpr IntIterator<T> end() const { return { _lim, _step }; }
};

template<typename T>
constexpr IntRange<T>
irange(T lim)
{
    return { 0, 1, lim };
}

template<typename T, typename U>
constexpr auto
irange(T start, U lim)
{
    using V = std::decay_t<decltype(start + lim)>;
    return IntRange<V>{ static_cast<V>(start), 1, static_cast<V>(lim) };
}

template<typename T = size_t>
IntRange<T>
irange()
{
    return irange(std::numeric_limits<T>::max());
}

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

    constexpr std::pair<Idx, typename std::iterator_traits<It>::reference>
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
constexpr auto
enumerate(T &&coll)
{
    using Iter = std::decay_t<decltype(coll.begin())>;
    return EnumeratedRange<Idx, Iter>{ coll.begin(), coll.end() };
}

#endif
