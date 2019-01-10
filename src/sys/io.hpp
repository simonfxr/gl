#ifndef SYS_IO_HPP
#define SYS_IO_HPP

#include "sys/io/Stream.hpp"

#include "sys/endian.hpp"

#include <memory>
#include <optional>
#include <string_view>

namespace sys {
namespace io {

struct Handle;
struct Socket;
struct IPAddr4;
struct HandleStream;

inline constexpr size_t HANDLE_READ_BUFFER_SIZE = 1024;
inline constexpr size_t HANDLE_WRITE_BUFFER_SIZE = 1024;

using HandleMode = uint32_t;

inline constexpr HandleMode HM_READ = 1;
inline constexpr HandleMode HM_WRITE = 2;
inline constexpr HandleMode HM_APPEND = 4;
inline constexpr HandleMode HM_NONBLOCKING = 8;

using SocketProto = uint32_t;

inline constexpr SocketProto SP_TCP = 1;

using SocketMode = uint32_t;

inline constexpr SocketMode SM_NONBLOCKING = 1;

SYS_API const IPAddr4
IPA_ANY();

SYS_API const IPAddr4
IPA_LOCAL();

enum HandleError
{
    HE_OK,
    HE_BLOCKED,
    HE_EOF,
    HE_BAD_HANDLE,
    HE_INVALID_PARAM,
    HE_UNKNOWN
};

struct SYS_API IPAddr4
{
    uint32_t addr4; // bigendian/network byte order
    constexpr IPAddr4() : addr4(0) {}
    constexpr IPAddr4(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
      : addr4(hton((uint32_t(a) << 24) | (uint32_t(b) << 16) |
                   (uint32_t(c) << 8) | uint32_t(d)))
    {}

    explicit constexpr IPAddr4(uint32_t addr) : addr4(addr) {}
};

} // namespace io
} // namespace sys

#if HU_OS_POSIX_P
#include "sys/io/io_unix.hpp"
#elif HU_OS_WINDOWS_P
#include "sys/io/io_windows.hpp"
#else
#error "no IO implementation available"
#endif

namespace sys {
namespace io {

SYS_API Handle
stdin_handle();

SYS_API Handle
stdout_handle();

SYS_API Handle
stderr_handle();

SYS_API HandleError
open(std::string_view, HandleMode, Handle *);

SYS_API HandleMode
mode(Handle &);

SYS_API HandleError
elevate(Handle &, HandleMode);

SYS_API HandleError
read(Handle &, size_t &, char *);

SYS_API HandleError
write(Handle &, size_t &, const char *);

SYS_API HandleError
close(Handle &);

enum SocketError
{
    SE_OK,
    SE_BLOCKED,
    SE_EOF,
    SE_BAD_SOCKET,
    SE_INVALID_PARAM,
    SE_UNKNOWN
};

SYS_API SocketError
listen(SocketProto, const IPAddr4 &, uint16_t, SocketMode, Socket *);

SYS_API SocketError
accept(Socket &, Handle *);

SYS_API SocketError
close(Socket &);

struct SYS_API HandleStream : public IOStream
{
    Handle handle;
    char read_buffer[HANDLE_READ_BUFFER_SIZE]{};
    char write_buffer[HANDLE_WRITE_BUFFER_SIZE]{};
    size_t read_cursor;
    size_t write_cursor;

    HandleStream(HandleStream &&) = default;
    HandleStream(const HandleStream &) = default;

    HandleStream(const Handle &);
    ~HandleStream() override;

    HU_NODISCARD
    static std::optional<HandleStream> open(std::string_view path,
                                            HandleMode mode,
                                            HandleError &);

    HU_NODISCARD
    static std::optional<HandleStream> open(std::string_view path,
                                            HandleMode mode)
    {
        HandleError err;
        return open(path, mode, err);
    }

protected:
    StreamResult basic_close() final override;
    StreamResult basic_flush() final override;
    StreamResult basic_read(size_t &, char *) final override;
    StreamResult basic_write(size_t &, const char *) final override;
    StreamResult flush_buffer();
};

SYS_API std::pair<std::unique_ptr<char[]>, size_t>
readFile(sys::io::OutStream &err, std::string_view path) noexcept;

} // namespace io
} // namespace sys

#endif
