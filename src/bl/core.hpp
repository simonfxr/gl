#ifndef BL_CORE_HPP
#define BL_CORE_HPP

#include <bl/conf.hpp>

#include <type_traits>

namespace bl {

using nullptr_t = decltype(nullptr);

using max_align_t = ::max_align_t;

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

} // namespace bl

#endif // BL_CORE_HPP
