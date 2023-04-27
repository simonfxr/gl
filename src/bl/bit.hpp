#ifndef BL_BIT_HPP
#define BL_BIT_HPP

#include <bl/type_traits.hpp>

#include <bit>

namespace bl {

template<typename To, typename From>
    requires(sizeof(To) == sizeof(From) && std::is_trivially_copyable_v<From> &&
             std::is_trivially_copyable_v<To>)
BL_inline constexpr To bit_cast(const From &src) noexcept
{
    static_assert(std::is_trivially_constructible_v<To>,
                  "This implementation additionally requires "
                  "destination type to be trivially constructible");
    return std::bit_cast<To>(src);
}

} // namespace bl

#endif // BL_BIT_HPP
