#ifndef SYS_FIBER_HPP
#define SYS_FIBER_HPP

#include "bl/byteops.hpp"
#include "bl/new.hpp"
#include "sys/conf.hpp"

#ifndef HAVE_FIBER
#    error "HAVE_FIBER not defined"
#endif

#include <fiber/fiber.h>

namespace sys {

struct SYS_API Fiber
{
    Fiber(const Fiber &) = delete;
    ~Fiber();

#ifndef NDEBUG
    Fiber(Fiber &&f) noexcept { *this = std::move(f); }
#else
    Fiber(Fiber &&f) noexcept = default;
#endif

    Fiber &operator=(const Fiber &) = delete;

#ifndef NDEBUG
    Fiber &operator=(Fiber &&fbr) noexcept
    {
        _bare = fbr._bare;
        bl::memzero(&fbr._bare);
        return *this;
    }
#else
    Fiber &operator=(Fiber &&fbr) noexcept = default;
#endif

    bool is_toplevel() const { return ::fiber_is_toplevel(&_bare); }
    bool is_executing() const { return ::fiber_is_executing(&_bare); }
    bool is_alive() const { return ::fiber_is_alive(&_bare); }
    void set_is_alive(bool val) { ::fiber_set_alive(&_bare, val); }

    size_t stack_size() const { return ::fiber_stack_size(&_bare); }
    size_t stack_free_size() const { return ::fiber_stack_free_size(&_bare); }

    static Fiber &active() noexcept;

    static Fiber &toplevel() noexcept;

    static void switch_to(Fiber &) noexcept;

    template<typename F>
    static Fiber make(F &&f, size_t stack_size = 64 * 1024)
    {
        Fiber fbr;
        using EntryArgs_t = EntryArgs<std::decay_t<F>>;
        void *args0{};
        (void) fiber_alloc(
          &fbr._bare, stack_size, fiber_guard, nullptr, FIBER_FLAG_GUARD_LO);
        fiber_push_return(&fbr._bare, fiber_entry, &args0, sizeof(EntryArgs_t));
        auto args = static_cast<EntryArgs_t *>(args0);
        args->functorp = &args->functor;
        args->invoke = EntryArgs_t::run_invoke;
        new (bl::addressof(args->functor)) F(std::forward<F>(f));
        return fbr;
    }

private:
    struct BasicEntryArgs
    {
        void (*invoke)(void *) noexcept;
        void *functorp;
    };

    template<typename F>
    struct EntryArgs : BasicEntryArgs
    {
        union
        {
            F functor;
        };

        static void run_invoke(void *args0) noexcept
        {
            auto args = static_cast<EntryArgs *>(args0);
            args->functor();
            auto &cur = active();
            cur.set_is_alive(false);
            args->functor.~F();
            fiber_done();
        }
    };

    Fiber() = default;

    static void fiber_guard(::Fiber *, void *);
    static void fiber_entry(void *entry_args);

    HU_NORETURN static void fiber_done();

    ::Fiber _bare;

    static Fiber init_toplevel() noexcept
    {
        Fiber fiber;
        ::fiber_init_toplevel(&fiber._bare);
        return fiber;
    }
};

} // namespace sys

#endif
