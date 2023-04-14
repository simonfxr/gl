#ifndef BL_INVOKE_HPP
#define BL_INVOKE_HPP

#include <bl/concepts.hpp>

/**
 * tag_invoke implementation from P1895R0:
 * https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1895r0.pdf
 *
 * We shorten the name tag_invoke to just invoke, which we can afford since we
 * are not using the std namespace.
 */
namespace bl {

namespace __invoke_f_ns {
void
invoke() = delete;

struct invoke_f
{
    template<typename Tag, typename... Args>
        requires requires(Tag tag, Args &&...args) {
                     invoke((Tag &&) tag, (Args &&) args...);
                 }
    constexpr auto operator()(Tag tag, Args &&...args) const
      noexcept(noexcept(invoke((Tag &&) tag, (Args &&) args...)))
        -> decltype(invoke((Tag &&) tag, (Args &&) args...))
    {
        return invoke((Tag &&) tag, (Args &&) args...);
    }
};
} // namespace __invoke_f_ns

inline namespace __invoke_ns {
inline constexpr __invoke_f_ns::invoke_f invoke = {};
}

template<typename Tag, typename... Args>
concept tag_invocable = requires(Tag tag, Args... args) {
                            bl::invoke((Tag &&) tag, (Args &&) args...);
                        };

template<typename Tag, typename... Args>
concept nothrow_tag_invocable =
  tag_invocable<Tag, Args...> &&
  requires(Tag tag, Args... args) {
      {
          bl::invoke((Tag &&) tag, (Args &&) args...)
      } noexcept;
  };

template<typename Tag, typename... Args>
using invoke_result = std::invoke_result<decltype(bl::invoke), Tag, Args...>;

template<typename Tag, typename... Args>
using invoke_result_t =
  std::invoke_result_t<decltype(bl::invoke), Tag, Args...>;

template<auto &Tag>
using tag_t = std::decay_t<decltype(Tag)>;

} // namespace bl

#endif // BL_INVOKE_HPP
