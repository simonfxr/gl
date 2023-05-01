#ifndef BL_TYPE_TRAITS_HPP
#define BL_TYPE_TRAITS_HPP

#include <bl/conf.hpp>

#include <type_traits>

namespace bl {

template<typename T>
struct always_false : std::false_type
{};

template<typename T>
using always_false_t = typename always_false<T>::type;

template<typename T>
inline constexpr bool always_false_v = always_false<T>::value;

} // namespace bl

#endif // BL_TYPE_TRAITS_HPP
