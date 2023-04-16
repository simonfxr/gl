#ifndef BL_INVOKE_HPP
#define BL_INVOKE_HPP

#include <bl/concepts.hpp>
#include <bl/core.hpp>

/**
 * tag_invoke implementation, see P1895R0:
 * https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1895r0.pdf
 *
 * code is adapted from:
 * https://github.com/facebookexperimental/libunifex
 *
 * We shorten the name tag_invoke to just invoke, which we can afford since we
 * are not using the std namespace.
 */
namespace bl {

BL_DIAG_PUSH
BL_DIAG_IGN_CLANG("-Wold-style-cast")
BL_DIAG_IGN_CLANG("-Wreserved-identifier")

namespace _invoke {
void
invoke();

struct _fn
{
    template<typename CPO, typename... Args>
    constexpr auto operator()(CPO cpo, Args &&...args) const
      noexcept(noexcept(invoke((CPO &&) cpo, (Args &&) args...)))
        -> decltype(invoke((CPO &&) cpo, (Args &&) args...))
    {
        return invoke((CPO &&) cpo, (Args &&) args...);
    }
};

template<typename CPO, typename... Args>
using invoke_result_t =
  decltype(invoke(bl_declval(CPO &&), bl_declval(Args &&)...));

using yes_type = char;
using no_type = char (&)[2];

template<typename CPO,
         typename... Args,
         typename = invoke_result_t<CPO, Args...>>
yes_type
try_invoke(int) //
  noexcept(noexcept(invoke(bl_declval(CPO &&), bl_declval(Args &&)...)));

template<typename CPO, typename... Args>
no_type
try_invoke(...) noexcept(false);

template<template<typename...> class T, typename... Args>
struct defer
{
    using type = T<Args...>;
};

struct empty
{};
} // namespace _invoke

namespace _invoke_cpo {
inline constexpr _invoke::_fn invoke{};
}
using namespace _invoke_cpo;

template<auto &CPO>
using tag_t = std::remove_cvref_t<decltype(CPO)>;

// Manually implement the traits here rather than defining them in terms of
// the corresponding std::invoke_result/is_invocable/is_nothrow_invocable
// traits to improve compile-times. We don't need all of the generality of the
// std:: traits and the invoke traits are used heavily through libunifex
// so optimising them for compile time makes a big difference.

using _invoke::invoke_result_t;

template<typename CPO, typename... Args>
inline constexpr bool is_invocable_v =
  (sizeof(_invoke::try_invoke<CPO, Args...>(0)) == sizeof(_invoke::yes_type));

template<typename CPO, typename... Args>
struct invoke_result
  : std::conditional_t<is_invocable_v<CPO, Args...>,
                       _invoke::defer<invoke_result_t, CPO, Args...>,
                       _invoke::empty>
{};

template<typename CPO, typename... Args>
using is_invocable = std::bool_constant<is_invocable_v<CPO, Args...>>;

template<typename CPO, typename... Args>
inline constexpr bool nothrow_invocable =
  noexcept(_invoke::try_invoke<CPO, Args...>(0));

template<typename CPO, typename... Args>
using is_nothrow_invocable =
  std::bool_constant<nothrow_invocable<CPO, Args...>>;

template<typename CPO, typename... Args>
concept invocable = (sizeof(_invoke::try_invoke<CPO, Args...>(0)) ==
                     sizeof(_invoke::yes_type));

} // namespace bl

BL_DIAG_POP

#endif // BL_INVOKE_HPP
