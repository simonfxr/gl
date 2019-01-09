#ifndef SYS_IO_WINDOWS_HPP
#define SYS_IO_WINDOWS_HPP

namespace sys {
namespace io {

struct W32_HANDLE;
struct W32_SOCKET;

struct Handle
{
    union
    {
        W32_HANDLE *handle;
        W32_SOCKET *socket;
    };
    bool is_handle;
};

struct Socket
{};

} // namespace io
} // namespace sys

#endif
