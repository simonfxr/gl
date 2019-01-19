#ifndef SYS_CLOCK_HPP
#define SYS_CLOCK_HPP

#include "sys/conf.hpp"

namespace sys {

HU_NODISCARD SYS_API double
queryTimer() noexcept;

SYS_API void
sleep(double secs) noexcept;

} // namespace sys

#endif
