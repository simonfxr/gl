#ifndef SYS_MODULE_HPP
#define SYS_MODULE_HPP

#include "sys/conf.hpp"
#include "sys/io/Stream.hpp"
#include "sys/io.hpp"
#include "sys/fiber.hpp"

#ifdef DEFINE_SYS_MODULE
#  define SYS_MODULE_ACCESS
#else
#  define SYS_MODULE_ACCESS const
#endif

namespace sys {

namespace io {

struct Streams {
    FileStream stdout;
    FileStream stderr;

    Streams();
};

struct IO {
    const IPAddr4 ipa_any;
    const IPAddr4 ipa_local;

    IO();
};

} // namespace io

struct Fibers {
    Fiber toplevel;
    
    Fibers();
};

struct Module {
    io::Streams io_streams;
    io::IO io;
    Fibers fibers;
};

extern Module * SYS_MODULE_ACCESS module;

#undef SYS_MODULE_ACCESS

} // namespace sys

#endif
