#ifndef SYS_IO_HPP
#define SYS_IO_HPP

#include "sys/io/Stream.hpp"

#include "pp/enum.hpp"
#include "sys/endian.hpp"
#include "util/Array.hpp"
#include "util/expected.hpp"

#include <bl/noncopymove.hpp>
#include <bl/optional.hpp>
#include <bl/string.hpp>
#include <bl/string_view.hpp>
#include <bl/swap.hpp>

namespace sys::io {

struct Handle;
struct IPAddr4;
struct HandleStream;

inline constexpr bl::usize HANDLE_READ_BUFFER_SIZE = 1024;
inline constexpr bl::usize HANDLE_WRITE_BUFFER_SIZE = 1024;

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

PP_DEF_ENUM(HandleError,
            uint8_t,
            OK,
            BLOCKED,
            EOF,
            BAD_HANDLE,
            INVALID_PARAM,
            UNKNOWN);

template<typename T>
using HandleResult = util::expected<T, HandleError>;

HU_NODISCARD SYS_API Handle
stdin_handle();

HU_NODISCARD SYS_API Handle
stdout_handle();

HU_NODISCARD SYS_API Handle
stderr_handle();

HU_NODISCARD SYS_API HandleResult<Handle> open(bl::string_view, HandleMode);

HU_NODISCARD SYS_API HandleError
elevate(Handle &, HandleMode);

HU_NODISCARD SYS_API HandleResult<Handle>
listen(SocketProto, const IPAddr4 &, uint16_t, SocketMode);

HU_NODISCARD SYS_API HandleResult<Handle>
accept(Handle &);

#if HU_OS_POSIX_P
using NativeHandle = int;
static inline constexpr NativeHandle INVALID_NATIVE_HANDLE = -1;
#elif HU_OS_WINDOWS_P
using NativeHandle = void *;
static inline constexpr NativeHandle INVALID_NATIVE_HANDLE = nullptr;
#else
#    error "OS not supported"
#endif

struct Handle : private bl::non_copyable
{
    constexpr Handle() = default;
    constexpr Handle(Handle &&rhs) noexcept { swap(*this, rhs); }

    Handle &operator=(Handle &&rhs) noexcept
    {
        auto tmp = Handle{ bl::move(rhs) };
        swap(*this, tmp);
        return *this;
    }

    constexpr explicit operator bool() const
    {
        return _native != INVALID_NATIVE_HANDLE;
    }

    constexpr friend void swap(Handle &lhs, Handle &rhs) noexcept
    {
        bl::swap(lhs._mode, rhs._mode);
        bl::swap(lhs._native, rhs._native);
    }

    static Handle fromNative(NativeHandle h, HandleMode mode = {})
    {
        return Handle{ h, mode };
    }

    [[nodiscard]] constexpr HandleMode mode() const noexcept { return _mode; }

    [[nodiscard]] constexpr NativeHandle native() const noexcept
    {
        return _native;
    }

    [[nodiscard]] constexpr NativeHandle takeNative() &&noexcept
    {
        auto h = bl::exchange(_native, INVALID_NATIVE_HANDLE);
        bl::exchange(_mode);
        return h;
    }

    SYS_API friend bl::pair<bl::usize, HandleError>
    tag_invoke(read_f, Handle &, bl::span<bl::byte>);

    SYS_API friend bl::pair<bl::usize, HandleError>
    tag_invoke(write_f, Handle &, bl::span<const bl::byte>);

    SYS_API friend HandleError tag_invoke(close_f, Handle &);

    inline ~Handle();

private:
    constexpr Handle(NativeHandle h, HandleMode m) : _native(h), _mode(m) {}
    NativeHandle _native{ INVALID_NATIVE_HANDLE };
    HandleMode _mode{};
};

Handle::~Handle()
{
    if (*this)
        close(*this);
}

struct SYS_API HandleStream : public IOStream
{
private:
    Handle handle;
    bl::byte read_buffer[HANDLE_READ_BUFFER_SIZE]{};
    bl::byte write_buffer[HANDLE_WRITE_BUFFER_SIZE]{};
    bl::usize read_cursor{};
    bl::usize write_cursor{};

public:
    HandleStream(HandleStream &&) = default;

    explicit HandleStream(Handle);
    ~HandleStream() override;

    HU_NODISCARD
    static HandleResult<HandleStream> open(bl::string_view path,
                                           HandleMode mode = HM_READ);

protected:
    StreamResult basic_close() final override;
    StreamResult basic_flush() final override;
    StreamResult basic_read(bl::usize &, bl::byte *) final override;
    StreamResult basic_write(bl::usize &, const bl::byte *) final override;
    StreamResult flush_buffer();
};

[[nodiscard]] SYS_API HandleResult<Array<bl::cbyte>>
readFile(bl::string_view path, sys::io::OutStream &errout = stderr()) noexcept;

} // namespace sys::io

#endif
