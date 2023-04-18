#ifndef BL_CORE_HPP
#define BL_CORE_HPP

#include <bl/conf.hpp>

#include <memory>
#include <type_traits>

namespace bl {

using nullptr_t = decltype(nullptr);

// using max_align_t = ::max_align_t;

template<typename T>
BL_wrapper constexpr T &&
forward(std::remove_reference_t<T> &x) noexcept
{
    return static_cast<T &&>(x);
}

template<typename T>
BL_wrapper constexpr T &&
forward(std::remove_reference_t<T> &&x) noexcept
{
    static_assert(!std::is_lvalue_reference_v<T>);
    return static_cast<T &&>(x);
}

#define bl_fwd(...) ::bl::forward<decltype(__VA_ARGS__)>(__VA_ARGS__)

template<typename T>
BL_wrapper constexpr std::remove_reference_t<T> &&
move(T &&x) noexcept
{
    return static_cast<std::remove_reference_t<T> &&>(x);
}

template<typename T, typename U = T>
BL_wrapper constexpr T
exchange(T &obj,
         U &&new_value) noexcept(std::is_nothrow_move_constructible_v<T>
                                   &&std::is_nothrow_assignable_v<T &, U>)
{
    T old_value = move(obj);
    obj = bl_fwd(new_value);
    return old_value;
}

template<typename T>
    requires(!std::is_array_v<T>)
BL_inline constexpr void destroy_at(T *loc)
{
    loc->~T();
}

template<typename T>
    requires(!std::is_array_v<T>)
BL_inline constexpr T *relocate_at(T *source, T *dest)
{
    std::uninitialized_move(dest, dest + 1, source);
    destroy_at(source);
    return std::launder(dest);
}

} // namespace bl

#endif // BL_CORE_HPP
