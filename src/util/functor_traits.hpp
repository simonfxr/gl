#ifndef UTIL_FUNCTOR_TRAITS_HPP
#define UTIL_FUNCTOR_TRAITS_HPP

#include "defs.h"

#include "bl/type_traits.hpp"
#include <utility>

namespace detail {
template<typename T>
struct functor_traits_impl
{};

template<typename Functor, typename Ret, typename... Args>
struct functor_traits_impl<Ret (Functor::*)(Args...) const>
{
    static inline constexpr size_t arity = sizeof...(Args);
    using result_type = Ret;

    template<size_t N>
    struct arg_type
    {
        using type = std::tuple_element_t<N, std::tuple<Args...>>;
    };
};

template<typename Ret, typename... Args>
struct functor_traits_impl<Ret (*)(Args...)>
{
    static inline constexpr size_t arity = sizeof...(Args);
    using result_type = Ret;

    template<size_t N>
    struct arg_type
    {
        using type = std::tuple_element_t<N, std::tuple<Args...>>;
    };
};

template<typename F, typename Ret, typename... Args>
inline auto
decay_stateless_functor(F f, Ret (F::*)(Args...) const)
{
    return static_cast<Ret (*)(Args...)>(f);
}

} // namespace detail

template<typename F>
struct functor_traits : detail::functor_traits_impl<decltype(&F::operator())>
{};

template<typename Ret, typename... Args>
struct functor_traits<Ret (*)(Args...)>
  : detail::functor_traits_impl<Ret (*)(Args...)>
{};

template<typename F, size_t i>
using functor_arg_type = typename functor_traits<F>::template arg_type<i>::type;

template<typename FTraits, size_t i>
using functor_traits_arg_type = typename FTraits::template arg_type<i>::type;

template<typename F, typename = bl::enable_if<std::is_empty_v<F>>>
auto
decay_stateless_functor(F f)
{
    return detail::decay_stateless_functor(f, &F::operator());
}

#endif
