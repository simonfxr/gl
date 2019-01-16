#ifndef DATA_ARRAY_VIEW_HPP
#define DATA_ARRAY_VIEW_HPP

#include "defs.hpp"
#include "err/err.hpp"

#include <type_traits>

template<typename T>
struct ArrayView
{
    size_t _size;
    T *_elems;

    template<typename U>
    constexpr ArrayView(const ArrayView<U> &a) : ArrayView(a.data(), a.size())
    {}

    constexpr ArrayView() : _size(0), _elems(nullptr) {}

    constexpr ArrayView(T *es, size_t s) : _size(s), _elems(es) {}

    constexpr ArrayView(const ArrayView &) = default;

    constexpr ArrayView(ArrayView &&) = default;

    constexpr size_t size() const { return _size; }

    constexpr T &operator[](size_t i) { return _elems[i]; }

    constexpr const T &operator[](size_t i) const { return _elems[i]; }

    ArrayView &operator=(const ArrayView &) = delete;
    ArrayView &operator=(ArrayView &&) = delete;

    constexpr T *data() const { return _elems; }

    constexpr T *begin() const { return _elems; }
    constexpr T *end() const { return _elems + _size; }

    ArrayView<T> slice(size_t start, size_t n)
    {
        ASSERT(start < size());
        ASSERT(start + n <= size());
        return { _elems + start, n };
    }

    ArrayView<T> drop(size_t n)
    {
        ASSERT(n <= size());
        return { _elems + n, _size - n };
    }
};

template<typename T>
auto
view_array(const T &a)
{
    auto *p = a.data();
    return ArrayView<std::remove_pointer_t<decltype(p)>>(p, a.size());
}

template<typename T>
auto
view_array(T &a)
{
    auto *p = a.data();
    return ArrayView<std::remove_pointer_t<decltype(p)>>(p, a.size());
}

#endif
