#ifndef SYS_MODULE_HPP
#define SYS_MODULE_HPP

#include "sys/conf.hpp"
#include "sys/fiber.hpp"
#include "sys/io.hpp"
#include "sys/io/Stream.hpp"

#include <bl/box.hpp>

namespace sys {

namespace io {

#ifdef HU_OS_WINDOWS
struct WS32Init
{
    WS32Init();
    ~WS32Init();
};
#endif

struct Streams
{
    BufferedInStream<Handle> stdin;
    BufferedOutStream<Handle> stdout;
    BufferedOutStream<Handle> stderr;

    Streams();
};

struct IO
{
    const IPAddr4 ipa_any;
    const IPAddr4 ipa_local;

    IO();
};

} // namespace io

struct Fibers
{
    Fiber toplevel{};
    Fibers();
};

struct Module
{
    io::Streams io_streams;
    io::IO io;
#ifdef HU_OS_WINDOWS
    io::WS32Init _ws32_init;
#endif
    Fibers fibers;
};

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
extern bl::box<Module> mod;

} // namespace sys

#endif
