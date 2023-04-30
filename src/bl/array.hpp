#ifndef BL_ARRAY_HPP
#define BL_ARRAY_HPP

#include <bl/conf.hpp>
#include <bl/core.hpp>

#include <array>
#include <cstddef>

namespace bl {

template<typename T, usize N>
struct array : public std::array<T, N>
{};

template<typename T, typename... U>
array(T, U...) -> array<T, 1 + sizeof...(U)>;

template<typename T, usize N>
constexpr array<std::remove_cv_t<T>, N>
to_array(T (&a)[N])
{
    return [&]<usize... I>(std::index_sequence<I...>)
    {
        return array<std::remove_cv_t<T>, N>{ a[I]... };
    }
    (std::make_index_sequence<N>{});
}

template<typename T, usize N>
constexpr array<std::remove_cv_t<T>, N>
to_array(T (&&a)[N])
{
    return [&]<usize... I>(std::index_sequence<I...>)
    {
        return array<std::remove_cv_t<T>, N>{ bl_mov(a[I])... };
    }
    (std::make_index_sequence<N>{});
}

} // namespace bl

#endif // BL_ARRAY_HPP
