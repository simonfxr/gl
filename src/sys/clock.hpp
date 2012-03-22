#ifndef SYS_CLOCK_HPP
#define SYS_CLOCK_HPP

#include "sys/conf.hpp"

namespace sys {

SYS_API double queryTimer();

SYS_API void sleep(double secs);

} // namespace sys

#endif
