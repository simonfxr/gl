#ifndef SYS_IO_UNIX_HPP
#define SYS_IO_UNIX_HPP

namespace sys {
namespace io {

typedef int FileDescriptor;

struct Handle
{
    HandleMode _mode{};
    FileDescriptor _fd = -1;

    constexpr Handle() = default;
    Handle(const Handle &) = delete;
    Handle(Handle &&h) { *this = std::move(h); }

    ~Handle()
    {
        if (*this)
            close(*this);
    }

    Handle &operator=(const Handle &) = delete;

    Handle &operator=(Handle &&h)
    {
        _mode = std::exchange(h._mode, 0);
        _fd = std::exchange(h._fd, -1);
        return *this;
    }

    constexpr explicit operator bool() const { return _fd != -1; }
};

struct Socket
{
    FileDescriptor _fd = -1;

    constexpr Socket() = default;
    Socket(const Socket &) = delete;
    Socket(Socket &&s) { *this = std::move(s); }

    ~Socket()
    {
        if (*this)
            close(*this);
    }

    Socket &operator=(const Socket &) = delete;

    Socket &operator=(Socket &&s)
    {
        _fd = std::exchange(s._fd, -1);
        return *this;
    }

    constexpr explicit operator bool() const { return _fd != -1; }
};

} // namespace io
} // namespace sys

#endif
