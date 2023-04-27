#ifndef UTIL_ARRAY_HPP
#define UTIL_ARRAY_HPP

#include "defs.h"

#include <bl/core.hpp>
#include <bl/span.hpp>
#include <bl/string_view.hpp>
#include <bl/type_traits.hpp>

#include <memory>
#include <new>

#if HU_COMP_GNULIKE_P || hu_has_attribute(malloc)
#    define ATTR_MALLOC __attribute__((malloc))
#else
#    define ATTR_MALLOC
#endif

namespace detail {
inline constexpr size_t
sat_fma(size_t a, size_t b, size_t c)
{
    return a > (size_t(-1) / 2 / b - c) ? size_t(-1) / 2 : a * b + c;
}
} // namespace detail

template<typename T>
ATTR_MALLOC inline T *
new_array_uninitialized(size_t n) noexcept
{
    if (!n)
        return nullptr;
    return static_cast<T *>(operator new[](
      detail::sat_fma(n, sizeof(T), 0),
      static_cast<std::align_val_t>(alignof(T))));
}

template<typename T, typename Arg, typename... Args>
ATTR_MALLOC inline T *
new_array(size_t n, Arg arg, Args... args)
{
    auto *p = new_array_uninitialized<T>(n);
    for (size_t i = 0; i < n; ++i)
        new (p + i) T(arg, args...);
    return p;
}

template<typename T>
ATTR_MALLOC inline T *
new_array(size_t n)
{
    auto *p = new_array_uninitialized<T>(n);
    if constexpr (!std::is_trivially_default_constructible_v<T>) {
        std::uninitialized_default_construct_n(p, n);
    }
    return p;
}

template<typename T>
inline void
delete_array(T *arr, size_t n)
{
    if (!n)
        return;

    if constexpr (!std::is_trivially_destructible_v<T>) {
        std::destroy_n(arr, n);
    }

#if defined(__cpp_sized_deallocation) && __cpp_sized_deallocation
    operator delete[](static_cast<void *>(arr),
                      n * sizeof(T),
                      static_cast<std::align_val_t>(alignof(T)));
#else
    operator delete[](static_cast<void *>(arr),
                      static_cast<std::align_val_t>(alignof(T)));
#endif
}

template<typename T, typename U>
ATTR_MALLOC T *
dup_array(const U *arr, size_t n)
{
    if (!n)
        return nullptr;
    auto dst = new_array_uninitialized<T>(n);
    std::uninitialized_copy_n(arr, n, dst);
    return dst;
}

template<typename T, typename Iter>
ATTR_MALLOC T *
dup_array(Iter it, Iter itend, size_t n)
{
    auto dst = new_array_uninitialized<T>(n);
    std::uninitialized_copy(bl::move(it), bl::move(itend), dst);
    return dst;
}

template<typename T>
struct Array
{
    explicit Array(size_t n) : _data(new_array<T>(n)), _size(n) {}

    constexpr Array() noexcept = default;

    Array(T *data, size_t n) : Array(bl::span<T>{ data, n }) {}

    template<typename U, size_t E>
    Array(bl::span<U, E> view)
    {
        *this = view;
    }

    Array(Array &&rhs) noexcept { swap(*this, rhs); }

    template<typename U>
    Array(const Array<U> &rhs)
    {
        *this = rhs.view();
    }

    Array(const Array<T> &rhs) { *this = rhs.view(); }

    ~Array() { clear(); }

    void clear()
    {
        delete_array(bl::exchange(_data, nullptr), bl::exchange(_size, 0));
    }

    template<typename Iter>
    void assign(Iter it, Iter itend)
    {
        auto n = std::distance(it, itend);
        assign(bl::move(it), bl::move(itend), n);
    }

    template<typename Iter>
    void assign(Iter it, Iter itend, size_t new_size)
    {
        auto n = static_cast<size_t>(std::distance(it, itend));
        if (n > new_size) {
            itend = it + new_size;
            n = new_size;
        }
        if (size() == new_size) {
            std::copy(bl::move(it), bl::move(itend), begin());
        } else {
            auto old = bl::move(*this);
            _size = new_size;
            _data = dup_array<T>(bl::move(it), bl::move(itend), new_size);
            if constexpr (!std::is_trivially_default_constructible_v<T>)
                if (new_size > n)
                    std::uninitialized_default_construct(begin() + n, end());
        }
    }

    template<typename U, size_t E>
    Array &operator=(bl::span<U, E> view)
    {
        assign(view.begin(), view.end());
        return *this;
    }

    template<typename U>
    Array &operator=(const Array<U> &rhs)
    {
        *this = rhs.view();
        return *this;
    }

    Array &operator=(Array &&rhs) noexcept
    {
        clear();
        swap(rhs, *this);
        return *this;
    }

    friend void swap(Array<T> &lhs, Array<T> &rhs)
    {
        using std::swap;
        swap(lhs._size, rhs._size);
        swap(lhs._data, rhs._data);
    }

    void resize(size_t n)
    {
        if (n == size())
            return;
        assign(std::make_move_iterator(begin()),
               std::make_move_iterator(begin() + n),
               n);
    }

    bl::span<const T> view() const { return bl::span<const T>{ _data, _size }; }

    bl::span<T> view() { return bl::span<T>{ _data, _size }; }

    operator bl::span<const T>() const { return view(); }

    operator bl::span<T>() { return view(); }

    HU_FORCE_INLINE const T &operator[](size_t i) const { return _data[i]; }

    HU_FORCE_INLINE T &operator[](size_t i) { return _data[i]; }

    HU_FORCE_INLINE constexpr bool empty() const { return _size != 0; }

    HU_FORCE_INLINE constexpr size_t size() const { return _size; }

    HU_FORCE_INLINE constexpr size_t size_bytes() const
    {
        return view().size_bytes();
    }

    HU_FORCE_INLINE const T *data() const { return _data; }
    HU_FORCE_INLINE T *data() { return _data; }

    HU_FORCE_INLINE const T *begin() const { return _data; }
    HU_FORCE_INLINE const T *end() const { return _data + _size; }

    HU_FORCE_INLINE T *begin() { return _data; }
    HU_FORCE_INLINE T *end() { return _data + _size; }

private:
    T *_data{};
    size_t _size{};
};

#endif
