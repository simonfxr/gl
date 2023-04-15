#ifndef BL_FUNCTOR_HPP
#define BL_FUNCTOR_HPP

#include <bl/core.hpp>
#include <bl/invoke.hpp>

namespace bl {
inline constexpr struct map_f
{
    template<typename R, typename... Ts>
        requires invocable<map_f, R, Ts...>
    BL_inline constexpr auto operator()(R &&r, Ts &&...ts) const
      noexcept(nothrow_invocable<map_f, R, Ts...>)
        -> bl::invoke_result_t<map_f, R, Ts...>
    {
        return ::bl::invoke(*this, bl_fwd(r), bl_fwd(ts)...);
    }
} map;

inline constexpr struct for_each_f
{
    template<typename R, typename... Ts>
        requires invocable<for_each_f, R, Ts...>
    BL_inline constexpr auto operator()(R &&r, Ts &&...ts) const
      noexcept(nothrow_invocable<for_each_f, R, Ts...>)
        -> bl::invoke_result_t<for_each_f, R, Ts...>
    {
        return ::bl::invoke(*this, bl_fwd(r), bl_fwd(ts)...);
    }
} for_each;

} // namespace bl

#endif // BL_FUNCTOR_HPP
