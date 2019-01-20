#ifndef SYS_FIBER_HPP
#define SYS_FIBER_HPP

#include "sys/conf.hpp"

#ifndef HAVE_FIBER
#    error "HAVE_FIBER not defined"
#endif

#include <fiber/fiber.h>

namespace sys {

typedef ::Fiber Fiber;

namespace fiber {

SYS_API Fiber *
toplevel();

} // namespace fiber

} // namespace sys

#endif
