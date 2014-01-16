#ifndef SYS_FIBER_HPP
#define SYS_FIBER_HPP

#include "sys/conf.hpp"

#if 0
#define FIBER_SHARED SHARED_IMPORT
#if PTR_BITS == 32
#  define FIBER_BITS32
#else
#  define FIBER_BITS64
#endif

#include <fiber.h>
#endif

#include <stddef.h>
#include <stdlib.h>

typedef uint32_t FiberState;

#define FS_EXECUTING ((FiberState) 1)
#define FS_TOPLEVEL ((FiberState) 2)
#define FS_ALIVE ((FiberState) 4)

typedef struct {
    FiberState state;
} Fiber;

typedef void (*FiberFunc)(void *);

#define FIBER_NYI ::abort()

inline Fiber *fiber_init(Fiber *, void *, size_t) {
    FIBER_NYI;
}

inline void fiber_init_toplevel(Fiber *fiber) {
    fiber->state = FS_EXECUTING | FS_TOPLEVEL | FS_ALIVE;
}

inline int fiber_alloc(Fiber *, size_t) {
    FIBER_NYI;
}

inline void fiber_destroy(Fiber *) {
    FIBER_NYI;
}

inline void fiber_switch(Fiber *, Fiber *) {
    FIBER_NYI;
}

inline void fiber_push_return(Fiber *, FiberFunc, const void *, size_t) {
    FIBER_NYI;
}

inline void fiber_reserve_return(Fiber *, FiberFunc, void **, size_t) {
    FIBER_NYI;
}

inline void fiber_exec_on(Fiber *, Fiber *, FiberFunc, void *, size_t) {
    FIBER_NYI;
}

static inline int fiber_is_toplevel(Fiber *fiber) {
    return (fiber->state & FS_TOPLEVEL) != 0;
}

static inline int fiber_is_executing(Fiber *fiber) {
    return (fiber->state & FS_EXECUTING) != 0;
}

static inline int fiber_is_alive(Fiber *fiber) {
    return (fiber->state & FS_ALIVE) != 0;
}

static inline void fiber_set_alive(Fiber *fiber, int alive) {
    if (alive)
        fiber->state |= FS_ALIVE;
    else
        fiber->state &= ~FS_ALIVE;
}

namespace sys {

typedef ::Fiber Fiber;

namespace fiber {

SYS_API Fiber *toplevel();

} // namespace fiber

} // namespace sys

#endif
