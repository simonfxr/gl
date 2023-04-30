#include "sys/fiber.hpp"
#include "sys/module.hpp"

namespace sys {

Fibers::Fibers()
{
    fiber_init_toplevel(&toplevel);
}

namespace fiber {

Fiber *
toplevel()
{
    return &mod->fibers.toplevel;
}

} // namespace fiber
} // namespace sys
