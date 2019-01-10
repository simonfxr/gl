#ifndef SYS_IO_WINDOWS_HPP
#define SYS_IO_WINDOWS_HPP

namespace sys {
namespace io {

struct Handle
{
    void *_handle{};
    HandleMode _mode{};
    bool _is_socket = false;

    constexpr Handle() = default;
    Handle(const Handle &) = delete;
    Handle(Handle &&h) { *this = std::move(h); }

    Handle &operator=(const Handle &) = delete;
    Handle &operator=(Handle &&h)
    {
        _handle = std::exchange(h._handle, nullptr);
        _mode = std::exchange(h._mode, 0);
        _is_socket = std::exchange(h._is_socket, false);
        return *this;
    }

    constexpr explicit operator bool() const { return _handle != nullptr; }
};

struct Socket
{
    void *_socket{};

    constexpr Socket() = default;
    Socket(const Socket &) = delete;
    Socket(Socket &&s) { *this = std::move(s); }

    Socket &operator=(const Socket &) = delete;

    Socket &operator=(Socket &&s)
    {
        _socket = std::exchange(s._socket, nullptr);
        return *this;
    }

    constexpr explicit operator bool() const { return _socket != nullptr; }
};

} // namespace io
} // namespace sys

#endif
