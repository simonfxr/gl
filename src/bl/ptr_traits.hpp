#ifndef BL_PTR_TRAITS_HPP
#define BL_PTR_TRAITS_HPP

#include <bl/conf.hpp>

#include <type_traits>

namespace bl {

template<typename T>
struct ptr_traits
{
    using value_type = typename T::value_type;
    using reference_type = typename T::reference_type;
    static inline constexpr bool nullable = T::nullable;
    static inline constexpr bool unique = T::unique;
};

template<typename T>
struct ptr_traits<T *>
{
    using value_type = T;
    using reference_type = T &;
    static inline constexpr bool nullable = true;
    static inline constexpr bool unique = false;
};

} // namespace bl

#endif // BL_PTR_TRAITS_HPP
