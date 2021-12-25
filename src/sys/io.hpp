#ifndef SYS_IO_HPP
#define SYS_IO_HPP

#include "sys/io/Stream.hpp"

#include "pp/enum.hpp"
#include "sys/endian.hpp"
#include "util/Array.hpp"
#include "util/noncopymove.hpp"

#include <optional>
#include <string>
#include <string_view>

namespace sys::io {

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

struct SYS_API IPAddr4
{
    uint32_t addr4{}; // bigendian/network byte order
    constexpr IPAddr4() = default;
    constexpr IPAddr4(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
      : addr4(hton((uint32_t(a) << 24) | (uint32_t(b) << 16) |
                   (uint32_t(c) << 8) | uint32_t(d)))
    {}
    explicit constexpr IPAddr4(uint32_t addr) : addr4(addr) {}
};

inline constexpr IPAddr4 IPA_ANY = { 0, 0, 0, 0 };
inline constexpr IPAddr4 IPA_LOCAL = { 127, 0, 0, 1 };

#define SYS_HANDLE_ERROR_ENUM_DEF(T, V0, V)                                    \
    T(HandleError,                                                             \
      uint8_t,                                                                 \
      V0(OK) V(BLOCKED) V(EOF) V(BAD_HANDLE) V(INVALID_PARAM) V(UNKNOWN))

PP_DEF_ENUM_WITH_API(SYS_API, SYS_HANDLE_ERROR_ENUM_DEF);

HU_NODISCARD SYS_API Handle
stdin_handle();

HU_NODISCARD SYS_API Handle
stdout_handle();

HU_NODISCARD SYS_API Handle
stderr_handle();

HU_NODISCARD SYS_API std::optional<Handle>
open(std::string_view, HandleMode, HandleError &);

HU_NODISCARD SYS_API HandleMode
mode(Handle &);

HU_NODISCARD SYS_API HandleError
elevate(Handle &, HandleMode);

HU_NODISCARD SYS_API HandleError
read(Handle &, size_t &, char *);

HU_NODISCARD SYS_API HandleError
write(Handle &, size_t &, const char *);

SYS_API HandleError
close(Handle &);

#define SYS_SOCKET_ERROR_ENUM_DEF(T, V0, V)                                    \
    T(SocketError,                                                             \
      uint8_t,                                                                 \
      V0(OK) V(BLOCKED) V(EOF) V(BAD_HANDLE) V(INVALID_PARAM) V(UNKNOWN))

PP_DEF_ENUM_WITH_API(SYS_API, SYS_SOCKET_ERROR_ENUM_DEF);

HU_NODISCARD SYS_API std::optional<Socket>
listen(SocketProto, const IPAddr4 &, uint16_t, SocketMode, SocketError &);

HU_NODISCARD SYS_API std::optional<Handle>
accept(Socket &, SocketError &);

SYS_API SocketError
close(Socket &);

#if HU_OS_POSIX_P
struct OSHandle
{
    int fd = -1;
    static inline constexpr OSHandle nil() { return {}; }
    HU_NODISCARD inline constexpr bool is_nil() const { return fd == -1; }
};

struct OSSocket
{
    int fd = -1;
    static inline constexpr OSSocket nil() { return {}; }
    HU_NODISCARD inline constexpr bool is_nil() const { return fd == -1; }
};

#elif HU_OS_WINDOWS_P
struct OSHandle
{
    void *handle{};
    bool is_socket{};
    static inline constexpr OSHandle nil() { return {}; }
    HU_NODISCARD inline constexpr bool is_nil() const { return !handle; }
};

struct OSSocket
{
    void *socket{};
    static inline constexpr OSSocket nil() { return {}; }
    HU_NODISCARD inline constexpr bool is_nil() const { return !socket; }
};
#else
#    error "OS not supported"
#endif

struct Handle : private NonCopyable
{
    HandleMode _mode{};
    OSHandle _os{};

    constexpr Handle() = default;
    Handle(Handle &&h) noexcept { *this = std::move(h); }

    ~Handle()
    {
        if (*this)
            close(*this);
    }

    Handle &operator=(Handle &&h) noexcept
    {
        _mode = std::exchange(h._mode, 0);
        _os = std::exchange(h._os, OSHandle::nil());
        return *this;
    }

    constexpr explicit operator bool() const { return !_os.is_nil(); }
};

struct Socket : NonCopyable
{
    OSSocket _os;

    constexpr Socket() = default;
    Socket(Socket &&s) noexcept { *this = std::move(s); }

    ~Socket()
    {
        if (*this)
            close(*this);
    }

    Socket &operator=(Socket &&s) noexcept
    {
        _os = std::exchange(s._os, OSSocket::nil());
        return *this;
    }

    constexpr explicit operator bool() const { return !_os.is_nil(); }
};

struct SYS_API HandleStream : public IOStream
{
    Handle handle;
    char read_buffer[HANDLE_READ_BUFFER_SIZE]{};
    char write_buffer[HANDLE_WRITE_BUFFER_SIZE]{};
    size_t read_cursor;
    size_t write_cursor;

    HandleStream(HandleStream &&) = default;

    HandleStream(Handle);
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

HU_NODISCARD SYS_API Array<char>
readFile(sys::io::OutStream &errout,
         std::string_view path,
         HandleError &err) noexcept;

HU_NODISCARD inline Array<char>
readFile(sys::io::OutStream &errout, std::string_view path) noexcept
{
    HandleError err;
    return readFile(errout, path, err);
}

} // namespace sys::io

#endif
