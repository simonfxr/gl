#ifndef BL_RANGE_HPP
#define BL_RANGE_HPP

#include "bl/core.hpp"
#include "bl/iterator.hpp"
#include "bl/limits.hpp"
#include "bl/type_traits.hpp"

namespace bl {
template<typename T>
struct IntIterator
{
    T _index;
    T _step;

    using difference_type = ptrdiff_t;
    using value_type = const T;
    using pointer = bl::add_pointer_t<const T>;
    using reference = const T &;
    using iterator_category = bl::random_access_iterator_tag;

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
inline constexpr IntRange<T>
irange(T lim)
{
    return { T{}, T{ 1 }, lim };
}

template<typename T, typename U>
inline constexpr auto
irange(T start, U lim)
{
    using V = bl::decay_t<decltype(start + lim)>;
    return IntRange<V>{ static_cast<V>(start), V{ 1 }, static_cast<V>(lim) };
}

template<typename T = size_t>
inline IntRange<T>
irange()
{
    return bl::irange(bl::numeric_limits<T>::max());
}

} // namespace bl
#endif
