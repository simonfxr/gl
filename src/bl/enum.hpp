#ifndef BL_ENUM_HPP
#define BL_ENUM_HPP

#include <bl/invoke.hpp>
#include <bl/mp.hpp>

namespace bl {

inline constexpr struct enum_count_f
{
    template<typename T>
        requires invocable<enum_count_f, T>
    BL_inline constexpr auto operator()(T &&enum_type) const
      noexcept(nothrow_invocable<enum_count_f, T>)
        -> invoke_result_t<enum_count_f, T>
    {
        return ::bl::invoke(*this, bl_fwd(enum_type));
    }

} enum_count;

template<typename T>
    requires invocable<enum_count_f, bl::type_proxy<T>>
BL_inline constexpr auto
enum_count_of() noexcept(nothrow_invocable<enum_count_f, T>)
{
    return enum_count(bl::type_proxy<T>{});
}

} // namespace bl

#endif // BL_ENUM_HPP
