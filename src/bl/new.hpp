#ifndef BL_NEW_HPP
#define BL_NEW_HPP

#include "defs.h"

#include "bl/type_traits.hpp"

#include <new>

#if HU_COMP_GNULIKE_P || hu_has_attribute(malloc)
#    define ATTR_MALLOC __attribute__((malloc))
#else
#    define ATTR_MALLOC
#endif

namespace bl {

namespace detail {
inline constexpr size_t
sat_fma(size_t a, size_t b, size_t c)
{
    return a > size_t(-1) / b - c ? size_t(-1) : a * b + c;
}
} // namespace detail

template<typename T>
ATTR_MALLOC inline T *
new_bare_uninitialized_array(size_t n) noexcept
{
    return static_cast<T *>(operator new[](detail::sat_fma(n, sizeof(T), 0)));
}

template<typename T>
ATTR_MALLOC inline T *
new_uninitialized_array(size_t n) noexcept
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
    auto *p = new_uninitialized_array<T>(n);
    for (size_t i = 0; i < n; ++i)
        new (p + i) T(arg, args...);
    return p;
}

template<typename T>
ATTR_MALLOC inline T *
new_array(size_t n)
{
    auto *p = new_uninitialized_array<T>(n);
    if constexpr (!std::is_trivially_default_constructible_v<T>) {
        for (size_t i = 0; i < n; ++i)
            new (p + i) T;
    }
    return p;
}

template<typename T, typename Arg, typename... Args>
ATTR_MALLOC inline T *
new_bare_array(size_t n, Arg arg, Args... args)
{
    auto *p = new_bare_uninitialized_array<T>(n);
    for (size_t i = 0; i < n; ++i)
        new (p + i) T(arg, args...);
    return p;
}

template<typename T>
ATTR_MALLOC inline T *
new_bare_array(size_t n)
{
    auto *p = new_bare_uninitialized_array<T>(n);
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

template<typename T>
inline void
delete_bare_array(T *arr)
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

template<typename T>
inline void
delete_uninitialized_array(T *arr)
{
    if constexpr (std::is_trivially_destructible_v<T>) {
        operator delete[](static_cast<void *>(arr));
    } else {
        constexpr size_t overhead =
          (sizeof(size_t) + alignof(T) - 1) / alignof(T) * alignof(T);
        auto *q = reinterpret_cast<char *>(arr);
        auto *p = q - overhead;
        operator delete[](static_cast<void *>(p));
    }
}

template<typename T, typename U>
ATTR_MALLOC T *
copy_array(const U *arr, size_t n)
{
    T *dst = new_uninitialized_array<T>(n);
    for (size_t i = 0; i < n; ++i)
        dst[i] = arr[i];
    return dst;
}

template<typename T, typename U>
ATTR_MALLOC T *
copy_bare_array(const U *arr, size_t n)
{
    T *dst = new_bare_uninitialized_array<T>(n);
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

} // namespace bl

#endif
