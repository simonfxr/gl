#ifndef SYS_IO_WINDOWS_HPP
#define SYS_IO_WINDOWS_HPP

namespace sys {
namespace io {

struct Handle
{
    void *_handle{};
    HandleMode _mode{};
    bool _is_socket = false;

    constexpr explicit operator bool() const { return _handle != nullptr; }
};

struct Socket
{
    void *_socket{};

    constexpr explicit operator bool() const { return _socket != nullptr; }
};

} // namespace io
} // namespace sys

#endif
