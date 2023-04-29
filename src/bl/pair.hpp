#ifndef BL_PAIR_HPP
#define BL_PAIR_HPP

#include <bl/conf.hpp>

#include <utility>

namespace bl {

template<typename T, typename U>
using pair = std::pair<T, U>;

} // namespace bl

#endif // BL_PAIR_HPP
