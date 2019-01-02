#ifndef SYS_IO_UNIX_HPP
#define SYS_IO_UNIX_HPP

namespace sys {
namespace io {

typedef int FileDescriptor;

struct Handle
{
    HandleMode mode;
    FileDescriptor fd;
    Handle() : mode(0), fd(-1) {}
};

struct Socket
{
    FileDescriptor socket;
    Socket() : socket() {}
};

} // namespace io
} // namespace sys

#endif
