#ifndef BL_OPTIONAL_HPP
#define BL_OPTIONAL_HPP

#include "defs.h"

#include <optional>

namespace bl {

template<typename T>
using optional = std::optional<T>;

inline constexpr auto nullopt = std::nullopt;

} // namespace bl

#endif
