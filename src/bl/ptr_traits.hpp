#ifndef BL_PTR_TRAITS_HPP
#define BL_PTR_TRAITS_HPP

#include <bl/concepts.hpp>
#include <bl/conf.hpp>

#include <type_traits>

namespace bl {

template<typename T>
    requires requires(T p) {
                 typename T::element_type;
                 {
                     *p
                     } -> std::same_as<typename T::element_type &>;
                 {
                     p.operator->()
                     } -> std::same_as<
                       std::add_pointer_t<typename T::element_type>>;
             }
struct ptr_traits
{
    using element_type = typename T::element_type;
    static inline constexpr bool is_nullable = T::is_nullable;
    static inline constexpr bool is_owned = T::is_owned;

    template<typename U>
    using rebind = typename T::template rebind<U>;
};

template<typename T>
struct ptr_traits<T *>
{
    using element_type = T;
    static inline constexpr bool is_nullable = true;
    static inline constexpr bool is_owned = false;
    template<typename U>
    using rebind = U *;
};

template<typename T>
concept a_ptr = requires(T p) { typename ptr_traits<T>::element_type; };

} // namespace bl

#endif // BL_PTR_TRAITS_HPP
