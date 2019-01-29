#ifndef BL_NEW_HPP
#define BL_NEW_HPP

#include "defs.h"

#include "bl/iterator.hpp"
#include "bl/type_traits.hpp"

#include <new>

#if HU_COMP_GNULIKE_P || hu_has_attribute(malloc)
#    define ATTR_MALLOC __attribute__((malloc))
#else
#    define ATTR_MALLOC
#endif

namespace bl {

template<class T>
FORCE_INLINE inline void
punned_destroy_at(void *p) noexcept
{
    static_assert(std::is_nothrow_destructible_v<T>);
    static_cast<T *>(p)->~T();
}

template<typename T>
using destroy_fun_ptr = void (*)(T *) noexcept;

template<typename T>
inline auto
get_destroy_fun_ptr()
{
    return &punned_destroy_at<T>;
}

template<typename T>
inline destroy_fun_ptr<void>
get_punned_destroy_fun_ptr()
{
    return reinterpret_cast<destroy_fun_ptr<void>>(get_destroy_fun_ptr<T>());
}

template<class T>
FORCE_INLINE inline void
destroy_at(T *p) noexcept
{
    static_assert(std::is_nothrow_destructible_v<T>);
    p->~T();
}

template<class ForwardIt>
inline void
destroy(ForwardIt first, ForwardIt last)
{
    using T = typename iterator_traits<ForwardIt>::value_type;
    if constexpr (!std::is_trivially_destructible_v<T>) {
        for (; first != last; ++first)
            destroy_at(addressof(*first));
    } else {
        UNUSED(first);
        UNUSED(last);
    }
}

namespace detail {
inline constexpr size_t
sat_fma(size_t a, size_t b, size_t c)
{
    return a > size_t(-1) / b - c ? size_t(-1) : a * b + c;
}
} // namespace detail

template<typename T>
ATTR_MALLOC inline T *
new_uninitialized_bare_array(size_t n) noexcept
{
    return static_cast<T *>(operator new[](detail::sat_fma(n, sizeof(T), 0)));
}

template<typename T, typename Arg, typename... Args>
ATTR_MALLOC inline T *
new_bare_array(size_t n, Arg arg, Args... args)
{
    auto *p = new_uninitialized_bare_array<T>(n);
    for (size_t i = 0; i < n; ++i)
        new (p + i) T(arg, args...);
    return p;
}

template<typename T>
ATTR_MALLOC inline T *
new_bare_array(size_t n)
{
    auto *p = new_uninitialized_bare_array<T>(n);
    if constexpr (!std::is_trivially_default_constructible_v<T>) {
        for (size_t i = 0; i < n; ++i)
            new (p + i) T;
    }
    return p;
}

template<typename T>
inline void
delete_uninitialized_bare_array(T *arr, size_t)
{
    operator delete[](static_cast<void *>(arr));
}

} // namespace bl

#endif
