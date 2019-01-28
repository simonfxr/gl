#ifndef BL_CORE_HPP
#define BL_CORE_HPP

#include "bl/config.hpp"

#ifdef __GLIBCXX__
#    include <bits/move.h>
#elif defined(_LIBCPP_VERSION)
#    include <type_traits>
#else
#    include <utility>
#endif

namespace bl {

struct assign_tag_t
{};

struct move_tag_t : assign_tag_t
{};
struct copy_tag_t : assign_tag_t
{};

template<typename T, typename U = T>
BL_inline constexpr T
exchange(T &place, U &&nval)
{
    T oval = std::move(place);
    place = std::forward<U>(nval);
    return oval;
}

template<typename T>
BL_inline constexpr const T &
min(const T &a, const T &b)
{
    return (b < a) ? b : a;
}

template<typename T>
BL_inline constexpr const T &
max(const T &a, const T &b)
{
    return (a < b) ? b : a;
}

template<typename T>
inline T
declval(); /* undefined */

template<typename T>
BL_inline constexpr T *
addressof(T &arg) noexcept
{
    return __builtin_addressof(arg);
}

template<typename T>
const T *
addressof(const T &&) = delete;

template<typename T, typename U>
inline constexpr T &
assign(copy_tag_t, T &dest, U &&src)
{
    return dest = std::forward<U>(src);
}

template<typename T, typename U>
inline constexpr T &
assign(move_tag_t, T &dest, U &&src)
{
    return dest = std::move(src);
}

template<typename T, typename U>
inline constexpr T &
initialize(copy_tag_t, T *dest, U &&src)
{
    return *new (dest) T(std::forward<U>(src));
}

template<typename T, typename U>
inline constexpr T &
initialize(move_tag_t, T *dest, U &&src)
{
    return *new (dest) T(std::move(src));
}

} // namespace bl

#endif
