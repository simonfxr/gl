#ifndef BL_TYPE_TRAITS_HPP
#define BL_TYPE_TRAITS_HPP

#include "defs.h"

#include <type_traits>

namespace bl {

template<bool cond, typename... Args>
using enable_if = std::enable_if<cond, Args...>;

template<bool cond, typename... Args>
using enable_if_t = typename bl::enable_if<cond, Args...>::type;

} // namespace bl

#endif
