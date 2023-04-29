#ifndef BL_OPTIONAL_HPP
#define BL_OPTIONAL_HPP

#include <bl/conf.hpp>

#include <optional>

namespace bl {

inline constexpr auto nullopt = std::nullopt;
using nullopt_t = decltype(nullopt);

template<typename T>
using optional = std::optional<T>;

} // namespace bl

#endif // BL_OPTIONAL_HPP
