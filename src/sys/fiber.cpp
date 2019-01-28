#include "sys/fiber.hpp"

#include "err/err.hpp"

namespace sys {

namespace {
Fiber *&
active_fiber() noexcept
{
    static thread_local Fiber *the_fiber = &Fiber::toplevel();
    return the_fiber;
}
} // namespace

Fiber::~Fiber() {}

Fiber &
Fiber::active() noexcept
{
    return *active_fiber();
}

Fiber &
Fiber::toplevel() noexcept
{
    BEGIN_NO_WARN_GLOBAL_DESTRUCTOR
    static thread_local Fiber toplevel_fiber = init_toplevel();
    END_NO_WARN_GLOBAL_DESTRUCTOR
    return toplevel_fiber;
}

void
Fiber::switch_to(Fiber &other) noexcept
{
    auto &active_var = active_fiber();
    auto &current = *active_var;
    active_var = bl::addressof(other);
    fiber_switch(&current._bare, &other._bare);
}

void
Fiber::fiber_guard(::Fiber *, void *)
{
    FATAL_ERR("fiber invoked after it finished");
}

void
Fiber::fiber_entry(void *args0)
{
    auto *args = static_cast<BasicEntryArgs *>(args0);
    args->invoke(args);
}

void
Fiber::fiber_done()
{
    Fiber &active = Fiber::active();
    Fiber &toplevel = Fiber::toplevel();
    ASSERT_ALWAYS(&active != &toplevel);
    active.set_is_alive(false);
    switch_to(toplevel);
    FATAL_ERR("fiber invoked after it finished");
}

} // namespace sys
