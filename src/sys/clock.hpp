#ifndef SYS_CLOCK_HPP
#define SYS_CLOCK_HPP

#include "sys/conf.hpp"

namespace sys {

SYS_API HU_NODISCARD double
queryTimer() noexcept;

SYS_API void
sleep(double secs) noexcept;

} // namespace sys

#endif
