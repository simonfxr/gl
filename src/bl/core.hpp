#ifndef BL_CORE_HPP
#define BL_CORE_HPP

#include "defs.h"

#define BL_inline HU_FORCE_INLINE inline

#ifdef __GLIBCXX__
#    include <bits/move.h>
#elif defined(_LIBCPP_VERSION)
#    include <type_traits>
#else
#    include <utility>
#endif

namespace bl {

template<typename T, typename U = T>
HU_FORCE_INLINE inline constexpr T
exchange(T &place, U &&nval)
{
    T oval = std::move(place);
    place = std::forward<U>(nval);
    return oval;
}

template<typename T>
HU_FORCE_INLINE inline constexpr const T &
min(const T &a, const T &b)
{
    return (b < a) ? b : a;
}

template<typename T>
HU_FORCE_INLINE inline constexpr const T &
max(const T &a, const T &b)
{
    return (a < b) ? b : a;
}

template<typename T>
inline T
declval(); /* undefined */

template<typename T>
constexpr T *
addressof(T &arg) noexcept
{
    return __builtin_addressof(arg);
}

template<typename T>
const T *
addressof(const T &&) = delete;

} // namespace bl

#endif
