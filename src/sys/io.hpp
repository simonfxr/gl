#ifndef SYS_IO_HPP
#define SYS_IO_HPP

#include "sys/conf.hpp"

#include "sys/endian.hpp"
#include "sys/io/Stream.hpp"

#include <memory>
#include <string>

namespace sys {

namespace io {

struct Handle;
struct Socket;
struct IPAddr4;
struct HandleStream;

namespace {

inline constexpr defs::size_t HANDLE_READ_BUFFER_SIZE = 1024;
inline constexpr defs::size_t HANDLE_WRITE_BUFFER_SIZE = 1024;

using HandleMode = uint32_t;

inline constexpr HandleMode HM_READ = 1;
inline constexpr HandleMode HM_WRITE = 2;
inline constexpr HandleMode HM_APPEND = 4;
inline constexpr HandleMode HM_NONBLOCKING = 8;

using SocketProto = uint32_t;

inline constexpr SocketProto SP_TCP = 1;

using SocketMode = uint32_t;

inline constexpr SocketMode SM_NONBLOCKING = 1;

} // namespace

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
    IPAddr4() : addr4(0) {}
    IPAddr4(defs::uint8_t a, defs::uint8_t b, defs::uint8_t c, defs::uint8_t d)
      : addr4(hton((uint32_t(a) << 24) | (uint32_t(b) << 16) |
                   (uint32_t(c) << 8) | uint32_t(d)))
    {}

    IPAddr4(uint32_t addr) : addr4(addr) {}
};

} // namespace io

} // namespace sys

#ifdef HU_OS_POSIX_P
#include "sys/io/io_unix.hpp"
#elif defined(HU_OS_WINDOWS_P)
#include "sys/io/io_windows.hpp"
#else
#error "no IO implementation available"
#endif

namespace sys {

namespace io {

SYS_API HandleError
open(const std::string &, HandleMode, Handle *);

SYS_API HandleMode
mode(Handle &);

SYS_API HandleError
elevate(Handle &, HandleMode);

SYS_API HandleError
read(Handle &, defs::size_t &, char *);

SYS_API HandleError
write(Handle &, defs::size_t &, const char *);

SYS_API HandleError
close(Handle &);

enum SocketError
{
    SE_OK,
    SE_BLOCKED,
    SE_BAD_SOCKET,
    SE_INVALID_PARAM,
    SE_UNKNOWN
};

SYS_API SocketError
listen(SocketProto, const IPAddr4 &, defs::uint16_t, SocketMode, Socket *);

SYS_API SocketError
accept(Socket &, Handle *);

SYS_API SocketError
close(Socket &);

struct SYS_API HandleStream : public IOStream
{
    Handle handle;
    char read_buffer[HANDLE_READ_BUFFER_SIZE]{};
    char write_buffer[HANDLE_WRITE_BUFFER_SIZE]{};
    defs::index_t read_cursor;
    defs::index_t write_cursor;

    HandleStream(const Handle & = Handle());
    ~HandleStream() override;

protected:
    StreamResult basic_close() final override;
    StreamResult basic_flush() final override;
    StreamResult basic_read(defs::size_t &, char *) final override;
    StreamResult basic_write(defs::size_t &, const char *) final override;
    StreamResult flush_buffer();
};

SYS_API std::pair<std::unique_ptr<char[]>, defs::size_t>
readFile(sys::io::OutStream &err, const std::string &path) noexcept;

} // namespace io

} // namespace sys

#endif
