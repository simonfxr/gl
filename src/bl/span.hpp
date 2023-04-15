#ifndef BL_SPAN_HPP
#define BL_SPAN_HPP

#include <bl/conf.hpp>

#include <limits>
#include <span>

namespace bl {

inline constexpr size_t dynamic_extent = std::numeric_limits<size_t>::max();

template<typename T, size_t Extent = dynamic_extent>
using span = std::span<T, Extent>;

} // namespace bl

#endif // BL_SPAN_HPP
