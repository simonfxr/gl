#ifndef BL_BYTEOPS_HPP
#define BL_BYTEOPS_HPP

#include "bl/core.hpp"

#if HU_COMP_GNULIKE_P
#    define BL_HAVE_BUILTIN_MEMCPY_P 1
#    define BL_HAVE_BUILTIN_MEMOVE_P 1
#    define BL_MEMCPY_CONSTEXPR_P HU_COMP_GCC_P
#    define BL_MEMMOVE_CONSTEXPR_P HU_COMP_GCC_P
#    define BL_HAVE_BUILTIN_MEMSET_P 1
#else
#    define BL_HAVE_BUILTIN_MEMCPY_P 0
#    define BL_MEMCPY_CONSTEXPR_P 0
#    define BL_HAVE_BUILTIN_MEMMOVE_P 0
#    define BL_MEMMOVE_CONSTEXPR_P 0
#    define BL_HAVE_BUILTIN_MEMSET_P 0
extern "C" void *BL_LIBC_CCONV
memcpy(void *BL_restrict, const void *BL_restrict, size_t) noexcept;

extern "C" void *BL_LIBC_CCONV
memmove(void *, const void *, size_t) noexcept;

extern "C" void *BL_LIBC_CCONV
memmove(void *, int, size_t) noexcept;

#endif

#define BL_MEMCPY_CONSTEXPR BL_CONSTEXPR_IF(BL_MEMCPY_CONSTEXPR_P)
#define BL_MEMMOVE_CONSTEXPR BL_CONSTEXPR_IF(BL_MEMMOVE_CONSTEXPR_P)
#define BL_MEMSET_CONSTEXPR BL_CONSTEXPR_IF(BL_HAVE_BUILTIN_MEMSET_P)

namespace bl {

template<typename T, typename U>
BL_inline BL_MEMCPY_CONSTEXPR void
memcpy(T *BL_restrict dst, const U *BL_restrict src, size_t n) noexcept
{
    BL_SELECT(BL_HAVE_BUILTIN_MEMCPY_P, __builtin_memcpy, ::memcpy)
    (dst, src, n);
}

template<typename T, typename U>
BL_inline BL_MEMMOVE_CONSTEXPR void
memmove(T *dst, const U *src, size_t n) noexcept
{
    BL_SELECT(BL_HAVE_BUILTIN_MEMCPY_P, __builtin_memmove, ::memcpy)
    (dst, src, n);
}

template<typename T>
BL_inline BL_MEMSET_CONSTEXPR void
memset(T *dst, uint8_t x, size_t n = sizeof(T)) noexcept
{
    BL_SELECT(BL_HAVE_BUILTIN_MEMCPY_P, __builtin_memset, ::memset)
    (dst, x, n);
}

template<typename T>
BL_inline BL_MEMSET_CONSTEXPR void
memzero(T *dst, size_t n = sizeof(T)) noexcept
{
    ::bl::memset(dst, 0, n);
}

template<class To, class From>
BL_inline BL_MEMCPY_CONSTEXPR
  bl::enable_if_t<(sizeof(To) == sizeof(From)) &&
                    bl::is_trivially_copyable_v<From> && bl::is_trivial_v<To>,
                  To>
  bit_cast(const From &src) noexcept
{
    To dst;
    ::bl::memcpy(&dst, &src, sizeof(To));
    return dst;
}
} // namespace bl

#endif
