#ifndef BL_CORE_HPP
#define BL_CORE_HPP

#include "defs.h"

#include "bl/type_traits.hpp"

namespace bl {

template<class T>
HU_FORCE_INLINE inline constexpr std::remove_reference_t<T> &&
move(T &&x) noexcept
{
    return static_cast<std::remove_reference_t<T> &&>(x);
}

template<class T>
HU_FORCE_INLINE inline constexpr T &&
forward(std::remove_reference_t<T> &x) noexcept
{
    return static_cast<T &&>(x);
}

template<class T>
HU_FORCE_INLINE inline constexpr T &&
forward(std::remove_reference_t<T> &&x) noexcept
{
    static_assert(!std::is_lvalue_reference_v<T>);
    return static_cast<T &&>(x);
}

template<typename T, typename U = T>
HU_FORCE_INLINE inline constexpr T
exchange(T &place, U &&nval)
{
    T oval = move(place);
    place = forward<U>(nval);
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

} // namespace bl

#endif
