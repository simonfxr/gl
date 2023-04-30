#ifndef BL_SWAP_HPP
#define BL_SWAP_HPP

#include <bl/invoke.hpp>
#include <bl/type_traits.hpp>

#include <utility>

namespace bl {

namespace _swap_internal {
using ::std::swap;

template<typename T, typename U>
inline constexpr bool is_swappable_with_v = std::is_swappable_with_v<T, U>;

template<typename T, typename U>
inline constexpr bool is_nothrow_swappable_with_v =
  std::is_nothrow_swappable_with_v<T, U>;
} // namespace _swap_internal

inline constexpr struct swap_f
{
    template<typename T, typename U>
        requires invocable<swap_f, T, U>
    BL_inline constexpr void operator()(T &&lhs, U &&rhs) const
      noexcept(nothrow_invocable<swap_f, T, U>)
    {
        ::bl::invoke(*this, bl_fwd(lhs), bl_fwd(rhs));
    }

    template<typename T, typename U>
        requires(_swap_internal::is_swappable_with_v<T &, U &>)
    BL_inline friend constexpr void invoke(swap_f, T &lhs, U &rhs) noexcept(
      _swap_internal::is_nothrow_swappable_with_v<T &, U &>)
    {
        using std::swap;
        swap(lhs, rhs);
    }
} swap;

} // namespace bl

#endif // BL_SWAP_HPP
