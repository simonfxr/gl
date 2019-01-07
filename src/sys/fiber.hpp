#ifndef SYS_FIBER_HPP
#define SYS_FIBER_HPP

#include "sys/conf.hpp"

#ifdef HAVE_FIBER
#include <fiber/fiber.h>
#else

#include <exception>

typedef uint32_t FiberState;

#define FS_EXECUTING ::FiberState(1)
#define FS_TOPLEVEL ::FiberState(2)
#define FS_ALIVE ::FiberState(4)

typedef struct
{
    FiberState state;
} Fiber;

typedef void (*FiberFunc)(void *);
typedef void (*FiberCleanupFunc)(Fiber *, void *);

#define FIBER_NYI std::terminate()
#define FIBER_NYI_RET(x)                                                       \
    FIBER_NYI;                                                                 \
    return x

inline Fiber *
fiber_init(Fiber *, void *, size_t, FiberCleanupFunc, void *)
{
    FIBER_NYI_RET(nullptr);
}

inline void
fiber_init_toplevel(Fiber *fiber)
{
    fiber->state = FS_EXECUTING | FS_TOPLEVEL | FS_ALIVE;
}

inline bool
fiber_alloc(Fiber *, size_t, FiberCleanupFunc, void *, bool)
{
#if HU_COMP_MSVC_P
    FIBER_NYI;
#else
	FIBER_NYI_RET(0);
#endif
}

inline void
fiber_destroy(Fiber *)
{
    FIBER_NYI;
}

inline void
fiber_switch(Fiber *, Fiber *)
{
    FIBER_NYI;
}

inline void
fiber_push_return(Fiber *, FiberFunc, const void *, size_t)
{
    FIBER_NYI;
}

inline void
fiber_reserve_return(Fiber *, FiberFunc, void **, size_t)
{
    FIBER_NYI;
}

inline void
fiber_exec_on(Fiber *, Fiber *, FiberFunc, void *, size_t)
{
    FIBER_NYI;
}

static inline int
fiber_is_toplevel(Fiber *fiber)
{
    return (fiber->state & FS_TOPLEVEL) != 0;
}

static inline int
fiber_is_executing(Fiber *fiber)
{
    return (fiber->state & FS_EXECUTING) != 0;
}

static inline int
fiber_is_alive(Fiber *fiber)
{
    return (fiber->state & FS_ALIVE) != 0;
}

static inline void
fiber_set_alive(Fiber *fiber, int alive)
{
    if (alive)
        fiber->state |= FS_ALIVE;
    else
        fiber->state &= ~FS_ALIVE;
}
#endif

namespace sys {

typedef ::Fiber Fiber;

namespace fiber {

SYS_API Fiber *
toplevel();

} // namespace fiber

} // namespace sys

#endif
