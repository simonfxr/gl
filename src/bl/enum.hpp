#ifndef BL_ENUM_HPP
#define BL_ENUM_HPP

#include <bl/mp.hpp>
#include <bl/tag_invoke.hpp>

namespace bl {

inline constexpr struct enum_count_f
{
    BL_DEF_TAG_CPO_OPERATOR(enum_count_f)
} enum_count;

template<typename T>
    requires tag_invocable<enum_count_f, bl::type_proxy<T>>
BL_inline constexpr auto
enum_count_of() noexcept(nothrow_tag_invocable<enum_count_f, T>)
{
    return enum_count(bl::type_proxy<T>{});
}

} // namespace bl

#endif // BL_ENUM_HPP
