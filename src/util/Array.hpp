#ifndef UTIL_ARRAY_HPP
#define UTIL_ARRAY_HPP

#include "util/ArrayView.hpp"

#include <new>
#include <type_traits>

#if HU_COMP_GNULIKE_P || hu_has_attribute(malloc)
#    define ATTR_MALLOC __attribute__((malloc))
#else
#    define ATTR_MALLOC
#endif

namespace detail {
inline constexpr size_t
sat_fma(size_t a, size_t b, size_t c)
{
    return a > size_t(-1) / b - c ? size_t(-1) : a * b + c;
}
} // namespace detail

template<typename T>
ATTR_MALLOC inline T *
new_array_uninitialized(size_t n) noexcept
{
    if constexpr (std::is_trivially_destructible_v<T>) {
        return static_cast<T *>(operator new[](
          detail::sat_fma(n, sizeof(T), 0)));
    } else {
        static_assert(alignof(T) <= alignof(std::max_align_t));
        constexpr size_t overhead =
          (sizeof(n) + alignof(T) - 1) / alignof(T) * alignof(T);
        char *p = static_cast<char *>(operator new[](
          detail::sat_fma(n, sizeof(T), overhead)));
        char *q = p + overhead;
        *reinterpret_cast<size_t *>(p) = n;
        return reinterpret_cast<T *>(q);
    }
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
        for (size_t i = 0; i < n; ++i)
            new (p + i) T;
    }
    return p;
}

template<typename T>
inline void
delete_array(T *arr)
{
    if constexpr (std::is_trivially_destructible_v<T>) {
        operator delete[](static_cast<void *>(arr));
    } else {
        constexpr size_t overhead =
          (sizeof(size_t) + alignof(T) - 1) / alignof(T) * alignof(T);
        auto *q = reinterpret_cast<char *>(arr);
        auto *p = q - overhead;
        auto n = *reinterpret_cast<size_t *>(p);
        for (size_t i = 0; i < n; ++i)
            arr[i].~T();
        operator delete[](static_cast<void *>(p));
    }
}

template<typename T, typename U>
ATTR_MALLOC T *
copy_array(const U *arr, size_t n)
{
    T *dst = new_array_uninitialized<T>(n);
    for (size_t i = 0; i < n; ++i)
        dst[i] = arr[i];
    return dst;
}

template<typename T, typename U>
void
copy_array_data(T *HU_RESTRICT dst, const U *HU_RESTRICT src, size_t n)
{
    for (size_t i = 0; i < n; ++i)
        dst[i] = src[i];
}

#include <vector>

template<typename T>
struct Array : private std::vector<T>
{
    using base = std::vector<T>;
    using std::vector<T>::vector;
    using base::begin;
    using base::data;
    using base::empty;
    using base::end;
    using base::size;
    using base::operator[];

    template<typename U = T>
    Array(const U *arr, size_t n) : base(arr, arr + n)
    {}

    template<typename U = T>
    Array(ArrayView<U> arr) : base(arr.begin(), arr.end())
    {}

    template<typename U>
    static Array<T> transfer_from(const U *data, size_t n)
    {
        return Array<T>(data, n);
    }
};

#if 0
template<typename T>
struct Array
{
    explicit Array(size_t n)
      : _data(n > 0 ? new_array<T>(n) : nullptr), _size(n)
    {}

    constexpr Array() noexcept = default;

    template<typename U>
    Array(ArrayView<U> view)
    {
        *this = view;
    }

    template<typename U>
    Array(Array<U> &&rhs) noexcept
    {
        *this = std::move(rhs);
    }

    template<typename U>
    Array(const Array<U> &rhs)
    {
        *this = rhs;
    }

    ~Array() { delete_array(_data); }

    void clear()
    {
        delete_array(_data);
        _data = nullptr;
        _size = 0;
    }

    template<typename U>
    Array &operator=(ArrayView<U> view)
    {
        if (_size == view.size()) {
            copy_array_data(_data, view.data(), view.size());
        } else {
            clear();
            if (view.size() != 0)
                _data = copy_array<T>(view.data(), view.size());
        }
        return *this;
    }

    template<typename U>
    Array &operator=(const Array<U> &rhs)
    {
        return *this = rhs.view();
    }

    template<typename U>
    Array &operator=(Array<U> &&rhs)
    {
        clear();
        _data = rhs._data;
        _size = rhs._size;
        rhs._data = nullptr;
        rhs._size = 0;
        return *this;
    }

    ArrayView<const T> view() const { return ArrayView<T>{ _data, _size }; }

    ArrayView<T> view() { return ArrayView<T>{ _data, _size }; }

    operator ArrayView<const T>() const { return view(); }

    operator ArrayView<T>() { return view(); }

    HU_FORCE_INLINE const T &operator[](size_t i) const { return _data[i]; }

    HU_FORCE_INLINE T &operator[](size_t i) { return _data[i]; }

    HU_FORCE_INLINE constexpr bool empty() const { return _size != 0; }

    HU_FORCE_INLINE constexpr size_t size() const { return _size; }

    HU_FORCE_INLINE const T *data() const { return _data; }
    HU_FORCE_INLINE T *data() { return _data; }

    HU_FORCE_INLINE const T *begin() const { return _data; }
    HU_FORCE_INLINE const T *end() const { return _data + _size; }

    HU_FORCE_INLINE T *begin() { return _data; }
    HU_FORCE_INLINE T *end() { return _data + _size; }

    static Array<T> transfer_from(T *data, size_t n)
    {
        return Array<T>(data, n);
    }

private:
    T *_data{};
    size_t _size{};

    Array(T *data, size_t n) : _data(data), _size(n) {}
};
#endif
#endif
