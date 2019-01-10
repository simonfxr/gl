#ifndef SYS_IO_UNIX_HPP
#define SYS_IO_UNIX_HPP

namespace sys {
namespace io {

typedef int FileDescriptor;

struct Handle
{
    HandleMode _mode{};
    FileDescriptor _fd = -1;

    constexpr explicit bool operator() const { return _fd != -1; }
};

struct Socket
{
    FileDescriptor _fd = -1;
    constexpr explicit bool operator() const { return _fd != -1; }
};

} // namespace io
} // namespace sys

#endif
