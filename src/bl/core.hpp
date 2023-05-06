#ifndef BL_CORE_HPP
#define BL_CORE_HPP

#include <bl/conf.hpp>
#include <bl/type_traits.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>

namespace bl {

using nullptr_t = decltype(nullptr);

using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using usize = std::size_t;
using isize = std::make_signed_t<usize>;

using byte = uint8_t;
using cbyte = char;

// using max_align_t = ::max_align_t;

using in_place_t = std::in_place_t;
inline constexpr in_place_t in_place{};

template<typename T>
using in_place_type_t = std::in_place_type_t<T>;

template<typename T>
inline constexpr in_place_type_t<T> in_place_type = in_place_type_t<T>{};

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

#define bl_mov(...) ::bl::move(__VA_ARGS__)

template<class T>
BL_wrapper constexpr T *
addressof(T &x) noexcept
{
#if hu_has_builtin(__builtin_addressof)
    return __builtin_addressof(x);
#else
    return std::addressoff(x);
#endif
}

template<typename T, typename U = T>
BL_wrapper constexpr T
exchange(T &obj,
         U &&new_value) noexcept(std::is_nothrow_move_constructible_v<T>
                                   &&std::is_nothrow_assignable_v<T &, U>)
{
    T old_value = ::bl::move(obj);
    obj = bl_fwd(new_value);
    return old_value;
}

template<typename T>
BL_wrapper constexpr T
exchange(T &obj) noexcept(std::is_nothrow_move_constructible_v<T>
                            &&std::is_nothrow_default_constructible_v<T>
                              &&std::is_nothrow_move_assignable_v<T>)
{
    T old_value = ::bl::move(obj);
    obj = T();
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
    std::uninitialized_move(source, source + 1, dest);
    ::bl::destroy_at(source);
    return std::launder(dest);
}

template<typename T, typename U = T>
BL_wrapper constexpr auto
min(T &&x, U &&y)
{
    return !(y < x) ? y : x;
}

template<typename T, typename U = T>
BL_wrapper constexpr auto
max(T &&x, U &&y)
{
    return !(y < x) ? x : y;
}

namespace literals {

BL_consteval usize operator""_z(unsigned long long x) noexcept
{
    return static_cast<usize>(x);
}

BL_consteval isize operator""_iz(unsigned long long x) noexcept
{
    return static_cast<isize>(x);
}

BL_consteval usize operator""_KiB(unsigned long long x) noexcept
{
    return static_cast<usize>(x) * 1024;
}

BL_consteval usize operator""_MiB(unsigned long long x) noexcept
{
    return static_cast<usize>(x) * 1024 * 1024;
}

BL_consteval usize operator""_GiB(unsigned long long x) noexcept
{
    return static_cast<usize>(x) * 1024 * 1024 * 1024;
}

} // namespace literals

} // namespace bl

#endif // BL_CORE_HPP
