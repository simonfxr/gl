#ifndef UTIL_FUNCTOR_TRAITS_HPP
#define UTIL_FUNCTOR_TRAITS_HPP

#include "defs.h"

#include <bl/tuple.hpp>
#include <bl/type_traits.hpp>

namespace util {

namespace _util_internal {

template<typename Ret, bool is_noexceptv, typename... Args>
struct functor_traits
{
    using result_type = Ret;
    using arguments = bl::tuple<Args...>;
    static inline constexpr size_t arity = sizeof...(Args);
    static inline constexpr bool is_noexcept = is_noexceptv;
    template<size_t I>
    struct arg_type
    {
        using type = bl::tuple_element_t<I, arguments>;
    };
    using fptr_type = Ret (*)(Args...) noexcept(is_noexcept);
};

template<typename Ret, typename... Args, bool is_noexcept>
auto functor_traits_for(Ret (*)(Args...) noexcept(is_noexcept))
  -> functor_traits<Ret, is_noexcept, Args...>;

template<typename Ret, typename F, typename... Args, bool is_noexcept>
auto functor_traits_for(Ret (F::*)(Args...) noexcept(is_noexcept))
  -> functor_traits<Ret, is_noexcept, Args...>;

template<typename Ret, typename F, typename... Args, bool is_noexcept>
auto
functor_traits_for(Ret (F::*)(Args...) const noexcept(is_noexcept))
  -> functor_traits<Ret, is_noexcept, Args...>;

template<typename Ret, typename F, typename... Args, bool is_noexcept>
auto functor_traits_with(F, Ret (F::*)(Args...) noexcept(is_noexcept))
  -> functor_traits<Ret, is_noexcept, Args...>;

template<typename Ret, typename F, typename... Args, bool is_noexcept>
auto
functor_traits_with(F, Ret (F::*)(Args...) const noexcept(is_noexcept))
  -> functor_traits<Ret, is_noexcept, Args...>;

template<typename F>
    requires(!std::is_pointer_v<F> && !std::is_member_pointer_v<F>)
auto functor_traits_for(F f)
  -> decltype(functor_traits_with(bl_declval(F), &F::operator()));

} // namespace _util_internal

template<typename F>
using functor_traits =
  decltype(_util_internal::functor_traits_for(bl_declval(F)));

template<typename Traits, size_t I>
using functor_traits_arg_type = typename Traits::template arg_type<I>::type;

template<typename F, size_t I>
using functor_arg_type = functor_traits_arg_type<functor_traits<F>, I>;

template<typename F>
BL_inline constexpr auto
decay_stateless_functor(F &&f) noexcept
{
    return static_cast<typename functor_traits<std::decay_t<F>>::fptr_type>(f);
}

} // namespace util

#endif // UTIL_FUNCTOR_TRAITS_HPP
