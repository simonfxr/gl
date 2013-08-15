#ifndef SYS_FIBER_HPP
#define SYS_FIBER_HPP

#include "sys/conf.hpp"

#define FIBER_SHARED SHARED_IMPORT
#if PTR_BITS == 32
#  define FIBER_BITS32
#else
#  define FIBER_BITS64
#endif

#include <fiber.h>

namespace sys {

typedef ::Fiber Fiber;

namespace fiber {

SYS_API Fiber *toplevel();

} // namespace fiber

} // namespace sys

#endif
