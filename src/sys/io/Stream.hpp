#ifndef SYS_IO_STREAM_HPP
#define SYS_IO_STREAM_HPP

#include "sys/conf.hpp"

#include "pp/enum.hpp"
#include "sys/fiber.hpp"
#include "sys/io_operators.hpp"

#include <bl/any_unique.hpp>
#include <bl/array.hpp>
#include <bl/enum.hpp>
#include <bl/pair.hpp>
#include <bl/span.hpp>
#include <bl/string.hpp>
#include <bl/string_view.hpp>
#include <bl/tag_invoke.hpp>

#include <cstring>

#undef stdin
#undef stdout
#undef stderr
#undef EOF

namespace sys::io {

PP_DEF_ENUM(IOError,
            uint8_t,
            OK,
            BLOCKED,
            EOF,
            BAD_HANDLE,
            INVALID_PARAM,
            UNKNOWN);

template<typename T>
concept SomeInStream = bl::tag_invocable<read_f, T &, bl::mut_byte_span>;

using InStream = bl::any_ref_t<bl::overload<
  bl::pair<bl::usize, IOError>(bl::self &, bl::mut_byte_span)>(read)>;

using ClosableInStream = bl::any_ref_t<
  bl::overload<bl::pair<bl::usize, IOError>(bl::self &, bl::mut_byte_span)>(
    read),
  bl::overload<IOError(bl::self &)>(close)>;

template<typename T>
concept SomeOutStream = bl::tag_invocable<write_f, T &, bl::byte_span>;

using OutStream = bl::any_ref_t<
  bl::overload<bl::pair<bl::usize, IOError>(bl::self &, bl::byte_span)>(write)>;

template<typename T>
concept ClosableStream = bl::tag_invocable<close_f, T &>;

template<typename T>
concept FlushableStream = bl::tag_invocable<flush_f, T &>;

HU_NODISCARD SYS_API InStream
stdin();

HU_NODISCARD SYS_API OutStream
stdout();

HU_NODISCARD SYS_API OutStream
stderr();

struct SYS_API NullStream final
{
    NullStream();

    SYS_API friend bl::pair<bl::usize, IOError>
    tag_invoke(read_f, NullStream &, bl::mut_byte_span) noexcept;

    SYS_API friend bl::pair<bl::usize, IOError>
    tag_invoke(write_f, NullStream &, bl::byte_span) noexcept;
};

static_assert(std::convertible_to<NullStream &, OutStream>);
static_assert(std::convertible_to<NullStream &, InStream>);

struct SYS_API ByteStream final
{
    using C = bl::cbyte;
    explicit ByteStream(bl::usize bufsize = 32);
    explicit ByteStream(bl::basic_string_view<C> view);
    ByteStream(const ByteStream &) = default;
    ByteStream(ByteStream &&) noexcept = default;

    ~ByteStream();

    ByteStream &operator=(const ByteStream &) = default;
    ByteStream &operator=(ByteStream &&) noexcept = default;

    bl::usize size() const { return buffer.size(); }
    auto data() const { return buffer.data(); }
    auto data() { return buffer.data(); }
    [[nodiscard]] bl::basic_string<C> str() const &
    {
        return bl::basic_string<C>(data(), size());
    }
    bl::basic_string<C> str() && { return bl::move(buffer); }

    void reserve(bl::usize additional)
    {
        buffer.reserve(buffer.size() + additional);
    }

    operator bl::basic_string_view<C>() const { return { data(), size() }; }

    void truncate(bl::usize);

    SYS_API friend bl::pair<bl::usize, IOError>
    tag_invoke(read_f, ByteStream &, bl::mut_byte_span) noexcept;

    SYS_API friend bl::pair<bl::usize, IOError>
    tag_invoke(write_f, ByteStream &, bl::byte_span) noexcept;

private:
    bl::basic_string<C> buffer;
    bl::usize read_cursor;
};

static_assert(std::convertible_to<ByteStream &, OutStream>);
static_assert(std::convertible_to<ByteStream &, InStream>);

struct SYS_API CooperativeInStream final
{
    ClosableInStream in;
    Fiber *io_handler; // switched to on blocking reads
    Fiber *stream_user;

    CooperativeInStream(ClosableInStream in, Fiber *ioh, Fiber *su);

    SYS_API friend IOError tag_invoke(close_f, CooperativeInStream &) noexcept;

    SYS_API friend bl::pair<bl::usize, IOError>
    tag_invoke(read_f, CooperativeInStream &, bl::mut_byte_span) noexcept;
};

static_assert(std::convertible_to<CooperativeInStream &, InStream>);

#define DEF_OUTSTREAM_OP(T)                                                    \
    decltype(auto) operator<<(SomeOutStream auto &&out, T value)               \
    {                                                                          \
        write_repr(out, value);                                                \
        return out;                                                            \
    }

inline IOError
write_repr(OutStream out, bl::string_view str)
{
    auto [_, r] = write(out, bl::as_bytes(str));
    return r;
}

inline IOError
write_repr(OutStream out, char c)
{
    return write_repr(out, bl::string_view{ &c, 1 });
}

#define DEF_OPAQUE_OUTSTREAM_OP(T)                                             \
    SYS_API IOError write_repr(OutStream out, T x);                            \
    DEF_OUTSTREAM_OP(T)

DEF_OUTSTREAM_OP(char);
DEF_OUTSTREAM_OP(bl::string_view);

DEF_OPAQUE_OUTSTREAM_OP(short)
DEF_OPAQUE_OUTSTREAM_OP(unsigned short)
DEF_OPAQUE_OUTSTREAM_OP(int)
DEF_OPAQUE_OUTSTREAM_OP(unsigned)
DEF_OPAQUE_OUTSTREAM_OP(long)
DEF_OPAQUE_OUTSTREAM_OP(unsigned long)
DEF_OPAQUE_OUTSTREAM_OP(long long)
DEF_OPAQUE_OUTSTREAM_OP(unsigned long long)

DEF_OPAQUE_OUTSTREAM_OP(float);
DEF_OPAQUE_OUTSTREAM_OP(double);
DEF_OPAQUE_OUTSTREAM_OP(long double);

SYS_API IOError
write_repr(OutStream out, const void *x);

#undef DEF_OPAQUE_OUTSTREAM_OP
#undef DEF_OUTSTREAM_OP

template<std::same_as<bool> T>
decltype(auto)
operator<<(SomeOutStream auto &&out, T x)
{
    using namespace bl::literals;
    return out << (x ? BL_STRING_VIEW("true") : BL_STRING_VIEW("false"));
}

decltype(auto)
operator<<(SomeOutStream auto &&out, const bl::string &str)
{
    return out << bl::string_view{ str };
}

struct ptr
{
    const void *p;
};

decltype(auto)
operator<<(SomeOutStream auto &&out, const ptr &p)
{
    write_repr(out, p.p);
    return out;
}

struct SYS_API BufferedStreamBase final
{
    static bl::pair<bl::usize, IOError> read_from(bl::mut_byte_span,
                                                  bl::usize &,
                                                  InStream,
                                                  bl::mut_byte_span) noexcept;

    static bl::pair<bl::usize, IOError> write_to(bl::mut_byte_span,
                                                 bl::usize &,
                                                 bool line_buffered,
                                                 OutStream,
                                                 bl::byte_span) noexcept;

    static IOError flush_to(bl::mut_byte_span, bl::usize &, OutStream) noexcept;
};

template<SomeInStream Stream, typename Buffer = bl::array<bl::byte, 4096>>
struct SYS_API BufferedInStream final
{
    BufferedInStream(Stream &&s, Buffer buf) : _s(bl_fwd(s)), _buf(bl_fwd(buf))
    {}

    BufferedInStream(Stream &&s) : _s(bl_fwd(s)), _buf() {}

    SYS_API friend bl::pair<bl::usize, IOError>
    tag_invoke(read_f, BufferedInStream &, bl::mut_byte_span) noexcept;

    SYS_API friend IOError tag_invoke(close_f, BufferedInStream &s) noexcept
        requires(ClosableStream<Stream>)
    {
        return close(s._s);
    }

    SYS_API friend bl::pair<bl::usize, IOError>
    tag_invoke(read_f, BufferedInStream &s, bl::mut_byte_span buf) noexcept
    {
        return BufferedStreamBase::read_from(s._buf, s._cursor, s._s, buf);
    }

    const Stream &unbuffered() const noexcept { return _s; }
    Stream &unbuffered() noexcept { return _s; }

private:
    Stream _s;
    Buffer _buf;
    bl::usize _cursor;
};

template<SomeOutStream Stream, typename Buffer = bl::array<bl::byte, 4096>>
struct SYS_API BufferedOutStream final
{
    BufferedOutStream(Stream &&s, bool line_buffered, Buffer buf)
      : _s(bl_fwd(s)), _buf(bl_fwd(buf)), _line_buffered(line_buffered)
    {}

    BufferedOutStream(Stream &&s, bool line_buffered = false)
      : _s(bl_fwd(s)), _buf(), _line_buffered(line_buffered)
    {}

    SYS_API friend bl::pair<bl::usize, IOError>
    tag_invoke(read_f, BufferedOutStream &, bl::mut_byte_span) noexcept;

    SYS_API friend IOError tag_invoke(flush_f, BufferedOutStream &s) noexcept
    {
        auto err = BufferedStreamBase::flush_to(s._buf, s._cursor, s._s);
        if constexpr (FlushableStream<Stream>) {
            if (err == IOError::OK) {
                err = io::flush(s._s);
            }
        }
        return err;
    }

    SYS_API friend IOError tag_invoke(close_f, BufferedOutStream &s) noexcept
        requires(ClosableStream<Stream>)
    {
        auto err = io::flush(s);
        auto err2 = io::close(s._s);
        if (err != IOError::OK) {
            return err;
        }
        return err2;
    }

    SYS_API friend bl::pair<bl::usize, IOError>
    tag_invoke(write_f, BufferedOutStream &s, bl::byte_span buf) noexcept
    {
        return BufferedStreamBase::write_to(
          s._buf, s._cursor, s._line_buffered, s._s, buf);
    }

    const Stream &unbuffered() const noexcept { return _s; }
    Stream &unbuffered() noexcept { return _s; }

private:
    Stream _s;
    Buffer _buf;
    bl::usize _cursor{};
    bool _line_buffered;
};

} // namespace sys::io

#endif
