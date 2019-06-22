#ifndef BL_CORE_HPP
#define BL_CORE_HPP

#include "bl/config.hpp"
#include "bl/type_traits.hpp"

namespace bl {

using nullptr_t = std::nullptr_t;
using max_align_t = std::max_align_t;

template<typename T>
constexpr T &&
forward(bl::remove_reference_t<T> &x) noexcept
{
    return static_cast<T &&>(x);
}

template<typename T>
constexpr T &&
forward(bl::remove_reference_t<T> &&x) noexcept
{
    static_assert(!bl::is_lvalue_reference_v<T>);
    return static_cast<T &&>(x);
}

template<typename T>
constexpr bl::remove_reference_t<T> &&
move(T &&x) noexcept
{
    return static_cast<bl::remove_reference_t<T> &&>(x);
}

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
    T oval = bl::move(place);
    place = bl::forward<U>(nval);
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
    return dest = bl::forward<U>(src);
}

template<typename T, typename U>
inline constexpr T &
assign(move_tag_t, T &dest, U &&src)
{
    return dest = bl::move(src);
}

template<typename T, typename U>
inline constexpr T &
initialize(copy_tag_t, T *dest, U &&src)
{
    return *new (dest) T(bl::forward<U>(src));
}

template<typename T, typename U>
inline constexpr T &
initialize(move_tag_t, T *dest, U &&src)
{
    return *new (dest) T(bl::move(src));
}

} // namespace bl

#if 0
namespace std __attribute__((__visibility__("default")))
{
    template<typename _Tp>
    struct __move_if_noexcept_cond
      : public __and_<__not_<is_nothrow_move_constructible<_Tp>>,
                      is_copy_constructible<_Tp>>::type
    {};
    template<typename _Tp>
    constexpr
      typename conditional<__move_if_noexcept_cond<_Tp>::value,
                           const _Tp &,
                           _Tp &&>::type move_if_noexcept(_Tp & __x) noexcept
    {
        return bl::move(__x);
    }
    template<typename _Tp>
    inline constexpr _Tp *addressof(_Tp & __r) noexcept
    {
        return bl::__addressof(__r);
    }
    template<typename _Tp>
    const _Tp *addressof(const _Tp &&) = delete;
    template<typename _Tp, typename _Up = _Tp>
    inline _Tp __exchange(_Tp & __obj, _Up && __new_val)
    {
        _Tp __old_val = bl::move(__obj);
        __obj = bl::forward<_Up>(__new_val);
        return __old_val;
    }
    template<typename _Tp>
    inline typename enable_if<__and_<__not_<__is_tuple_like<_Tp>>,
                                     is_move_constructible<_Tp>,
                                     is_move_assignable<_Tp>>::value>::type
      swap(_Tp & __a,
           _Tp & __b) noexcept(__and_<is_nothrow_move_constructible<_Tp>,
                                      is_nothrow_move_assignable<_Tp>>::value)
    {

        _Tp __tmp = bl::move(__a);
        __a = bl::move(__b);
        __b = bl::move(__tmp);
    }
    template<typename _Tp, size_t _Nm>
    inline typename enable_if<__is_swappable<_Tp>::value>::type swap(
      _Tp(&__a)[_Nm],
      _Tp(&__b)[_Nm]) noexcept(__is_nothrow_swappable<_Tp>::value)
    {
        for (size_t __n = 0; __n < _Nm; ++__n)
            swap(__a[__n], __b[__n]);
    }

} // namespace std__attribute__((__visibility__("default")))
#endif

#endif
