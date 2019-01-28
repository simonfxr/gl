#ifndef BL_BYTEOPS_HPP
#define BL_BYTEOPS_HPP

#include "bl/core.hpp"

#if __has_builtin(__builtin_memcpy) || HU_COMP_GNULIKE_P
#    define BL_HAVE_BUILTIN_MEMCPY_P 1
#    define BL_HAVE_BUILTIN_MEMOVE_P 1
#    define BL_MEMCPY_CONSTEXPR_P HU_COMP_GCC_P
#    define BL_MEMMOVE_CONSTEXPR_P HU_COMP_GCC_P
#else
#    define BL_HAVE_BUILTIN_MEMCPY_P 0
#    define BL_MEMCPY_CONSTEXPR_P 0
#    define BL_HAVE_BUILTIN_MEMMOVE_P 0
#    define BL_MEMMOVE_CONSTEXPR_P 0
extern "C" void *BL_LIBC_CCONV
memcpy(void *BL_restrict, const void *BL_restrict, size_t) noexcept;

extern "C" void *BL_LIBC_CCONV
memmove(void *, const void *, size_t) noexcept;

#endif

#define BL_MEMCPY_CONSTEXPR BL_CONSTEXPR_IF(BL_MEMCPY_CONSTEXPR_P)
#define BL_MEMMOVE_CONSTEXPR BL_CONSTEXPR_IF(BL_MEMMOVE_CONSTEXPR_P)

namespace bl {

BL_inline BL_MEMCPY_CONSTEXPR void
memcpy(void *BL_restrict dst, const void *BL_restrict src, size_t n) noexcept
{
    BL_SELECT(BL_HAVE_BUILTIN_MEMCPY_P, __builtin_memcpy, ::memcpy)
    (dst, src, n);
}

BL_inline BL_MEMMOVE_CONSTEXPR void
memmove(void *dst, const void *src, size_t n) noexcept
{
    BL_SELECT(BL_HAVE_BUILTIN_MEMCPY_P, __builtin_memmove, ::memcpy)
    (dst, src, n);
}

template<class To, class From>
BL_inline BL_MEMCPY_CONSTEXPR std::enable_if_t<
  (sizeof(To) == sizeof(From)) && std::is_trivially_copyable_v<From> &&
    std::is_trivial_v<To>,
  To>
bit_cast(const From &src) noexcept
{
    To dst;
    memcpy(&dst, &src, sizeof(To));
    return dst;
}
} // namespace bl

#endif
