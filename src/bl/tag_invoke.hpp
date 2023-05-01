#ifndef BL_TAG_INVOKE_HPP
#define BL_TAG_INVOKE_HPP

#include <bl/concepts.hpp>
#include <bl/core.hpp>

/**
 * tag_invoke implementation, see P1895R0:
 * https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1895r0.pdf
 *
 * code is adapted from:
 * https://github.com/facebookexperimental/libunifex
 *
 */
namespace bl {

BL_DIAG_PUSH
BL_DIAG_IGN_CLANG("-Wold-style-cast")
BL_DIAG_IGN_CLANG("-Wreserved-identifier")
// NOLINTBEGIN(google-readability-casting)

namespace _tag_invoke {
void
tag_invoke();

struct _fn
{
    template<typename CPO, typename... Args>
    constexpr auto operator()(CPO cpo, Args &&...args) const
      noexcept(noexcept(tag_invoke((CPO &&) cpo, (Args &&) args...)))
        -> decltype(tag_invoke((CPO &&) cpo, (Args &&) args...))
    {
        return tag_invoke((CPO &&) cpo, (Args &&) args...);
    }
};

template<typename CPO, typename... Args>
using tag_invoke_result_t =
  decltype(tag_invoke(bl_declval(CPO &&), bl_declval(Args &&)...));

using yes_type = char;
using no_type = char (&)[2];

template<typename CPO,
         typename... Args,
         typename = tag_invoke_result_t<CPO, Args...>>
yes_type
try_tag_invoke(int) //
  noexcept(noexcept(tag_invoke(bl_declval(CPO &&), bl_declval(Args &&)...)));

template<typename CPO, typename... Args>
no_type
try_tag_invoke(...) noexcept(false);

template<template<typename...> class T, typename... Args>
struct defer
{
    using type = T<Args...>;
};

struct empty
{};
} // namespace _tag_invoke

namespace _tag_invoke_cpo {
inline constexpr _tag_invoke::_fn tag_invoke{};
}
using namespace _tag_invoke_cpo;

template<auto &CPO>
using tag_t = std::remove_cvref_t<decltype(CPO)>;

// Manually implement the traits here rather than defining them in terms of
// the corresponding
// tag_invoke_result/is_tag_invocable/is_nothrow_tag_invocable traits to
// improve compile-times. We don't need all of the generality of the std::
// traits and the tag_invoke traits are used heavily through libunifex so
// optimising them for compile time makes a big difference.

using _tag_invoke::tag_invoke_result_t;

template<typename CPO, typename... Args>
inline constexpr bool is_tag_invocable_v =
  (sizeof(_tag_invoke::try_tag_invoke<CPO, Args...>(0)) ==
   sizeof(_tag_invoke::yes_type));

template<typename CPO, typename... Args>
struct tag_invoke_result
  : std::conditional_t<is_tag_invocable_v<CPO, Args...>,
                       _tag_invoke::defer<tag_invoke_result_t, CPO, Args...>,
                       _tag_invoke::empty>
{};

template<typename CPO, typename... Args>
using is_tag_invocable = std::bool_constant<is_tag_invocable_v<CPO, Args...>>;

template<typename CPO, typename... Args>
inline constexpr bool nothrow_tag_invocable =
  noexcept(_tag_invoke::try_tag_invoke<CPO, Args...>(0));

template<typename CPO, typename... Args>
using is_nothrow_tag_invocable =
  std::bool_constant<nothrow_tag_invocable<CPO, Args...>>;

template<typename CPO, typename... Args>
concept tag_invocable = (sizeof(_tag_invoke::try_tag_invoke<CPO, Args...>(0)) ==
                         sizeof(_tag_invoke::yes_type));

} // namespace bl

// NOLINTEND(google-readability-casting)
BL_DIAG_POP

#define BL_DEF_TAG_CPO_OPERATOR(CPO_f)                                         \
    template<typename R, typename... Ts>                                       \
        requires ::bl::tag_invocable<CPO_f, R, Ts...>                          \
    BL_inline constexpr auto operator()(R &&r, Ts &&...ts)                     \
      const noexcept(::bl::nothrow_tag_invocable<CPO_f, R, Ts...>)             \
        ->::bl::tag_invoke_result_t<CPO_f, R, Ts...>                           \
    {                                                                          \
        return ::bl::tag_invoke(*this, bl_fwd(r), bl_fwd(ts)...);              \
    }

#endif // BL_TAG_INVOKE_HPP
