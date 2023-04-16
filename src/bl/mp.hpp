#ifndef BL_MP_HPP
#define BL_MP_HPP

#include <bl/concepts.hpp>

namespace bl {

struct basic_type_proxy
{};

template<typename T>
struct type_proxy : basic_type_proxy
{
    using type = T;
};

template<typename T>
concept a_type_proxy = std::derived_from<std::decay_t<T>, basic_type_proxy>;

template<a_type_proxy T>
using type_proxy_t = typename std::decay_t<T>::type;

} // namespace bl

#endif // BL_MP_HPP
