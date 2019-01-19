#ifndef UTIL_FUNCTOR_TRAITS_HPP
#define UTIL_FUNCTOR_TRAITS_HPP

#include "defs.h"

#include <type_traits>
#include <utility>

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

template<typename F>
struct functor_traits : functor_traits_impl<decltype(&F::operator())>
{};

template<typename Ret, typename... Args>
struct functor_traits<Ret (*)(Args...)> : functor_traits_impl<Ret (*)(Args...)>
{};

template<typename F, size_t i>
using functor_arg_type = typename functor_traits<F>::template arg_type<i>::type;

template<typename FTraits, size_t i>
using functor_traits_arg_type = typename FTraits::template arg_type<i>::type;

#endif
