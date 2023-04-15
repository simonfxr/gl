#ifndef BL_SWAP_HPP
#define BL_SWAP_HPP

#include <bl/invoke.hpp>

#include <utility>

namespace bl {

inline constexpr struct swap_f
{
    template<typename T, typename U>
        requires invocable<swap_f, T, U>
    BL_inline constexpr void operator()(T &&lhs, U &&rhs) const
      noexcept(nothrow_invocable<swap_f, T, U>)
    {
        return ::bl::invoke(*this, bl_fwd(lhs), bl_fwd(rhs));
    }

    template<typename T, typename U>
        requires std::swappable_with<T, U>
    BL_inline friend constexpr void invoke(swap_f, T &&lhs, U &&rhs) noexcept(
      std::is_nothrow_swappable_with<T, U>::value)
    {
        using std::swap;
        return swap(bl_fwd(lhs), bl_fwd(rhs));
    }
} swap;

} // namespace bl

#endif // BL_SWAP_HPP
