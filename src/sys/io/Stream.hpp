#ifndef SYS_IO_STREAM_HPP
#define SYS_IO_STREAM_HPP

#include "sys/conf.hpp"

#include "pp/enum.hpp"
#include "sys/fiber.hpp"
#include "sys/io_operators.hpp"

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

struct OutStream;
struct InStream;
struct IOStream;

HU_NODISCARD SYS_API InStream &
stdin();

HU_NODISCARD SYS_API OutStream &
stdout();

HU_NODISCARD SYS_API OutStream &
stderr();

PP_DEF_ENUM(StreamResult, uint8_t, OK, Blocked, EOF, Closed, Error);

using StreamFlags = uint16_t;

inline constexpr StreamFlags SF_LINEBUF = 1;

struct SYS_API InStream
{
    InStream() = default;
    InStream(const InStream &) = default;
    InStream(InStream &&) = default;
    InStream &operator=(const InStream &) = default;
    InStream &operator=(InStream &&) = default;

    virtual ~InStream();

    friend StreamResult tag_invoke(close_f, InStream &s)
    {
        return s.basic_close_in();
    }

    friend bl::pair<bl::usize, StreamResult> tag_invoke(read_f,
                                                        InStream &s,
                                                        bl::span<bl::byte> buf)
    {
        if (buf.empty())
            return { 0, StreamResult::OK };
        auto k = buf.size();
        auto err = s.basic_read(k, buf.data());
        return { k, err };
    }

protected:
    virtual StreamResult basic_read(bl::usize &s, bl::byte *buf) = 0;
    virtual StreamResult basic_close_in() = 0;
};

struct SYS_API OutStream
{
    OutStream() = default;
    OutStream(const OutStream &) = default;
    OutStream(OutStream &&) = default;
    OutStream &operator=(const OutStream &) = default;
    OutStream &operator=(OutStream &&) = default;

    virtual ~OutStream();

    [[nodiscard]] bool line_buffered() const { return wflags & SF_LINEBUF; }
    OutStream &line_buffered(bool yesno)
    {
        if (yesno)
            wflags |= SF_LINEBUF;
        else
            wflags &= ~SF_LINEBUF;
        return *this;
    }

    SYS_API friend bl::pair<bl::usize, StreamResult>
    tag_invoke(write_f, OutStream &o, bl::span<const bl::byte> buf);

    friend StreamResult tag_invoke(close_f, OutStream &o)
    {
        return o.basic_close_out();
    }

    friend StreamResult tag_invoke(flush_f, OutStream &o)
    {
        return o.basic_flush();
    }

protected:
    virtual StreamResult basic_write(bl::usize &s, const bl::byte *buf) = 0;
    virtual StreamResult basic_flush() = 0;
    virtual StreamResult basic_close_out() = 0;
    StreamFlags wflags{};
};

struct SYS_API IOStream
  : public InStream
  , public OutStream
{
    IOStream() = default;
    IOStream(const IOStream &) = default;
    IOStream(IOStream &&) = default;
    IOStream &operator=(const IOStream &) = default;
    IOStream &operator=(IOStream &&) = default;

    ~IOStream() override;

    friend StreamResult tag_invoke(close_f, IOStream &io)
    {
        return close(static_cast<OutStream &>(io));
    }

protected:
    StreamResult basic_close_out() final;
    StreamResult basic_close_in() final;

    virtual StreamResult basic_close() = 0;
};

struct SYS_API NullStream : public IOStream
{
    NullStream();

protected:
    StreamResult basic_flush() final;
    StreamResult basic_close() final;
    StreamResult basic_read(bl::usize &, bl::byte *) final;
    StreamResult basic_write(bl::usize &, const bl::byte *) final;
};

struct SYS_API ByteStream : public IOStream
{
    using C = bl::cbyte;
    explicit ByteStream(bl::usize bufsize = 32);
    explicit ByteStream(bl::basic_string_view<C> view);
    ByteStream(const ByteStream &) = default;
    ByteStream(ByteStream &&) noexcept = default;

    ~ByteStream() override;

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

protected:
    StreamResult basic_flush() final override;
    StreamResult basic_close() final override;
    StreamResult basic_read(bl::usize &, bl::byte *) final override;
    StreamResult basic_write(bl::usize &, const bl::byte *) final override;

private:
    bl::basic_string<C> buffer;
    bl::usize read_cursor;
};

struct SYS_API CooperativeInStream : public InStream
{
    InStream *in;
    Fiber *io_handler; // switched to on blocking reads
    Fiber *stream_user;

    CooperativeInStream(InStream *in, Fiber *ioh, Fiber *su);

protected:
    virtual StreamResult basic_close_in() final override;
    virtual StreamResult basic_read(bl::usize &, bl::byte *) final override;
};

template<typename OStream>
concept AOutStream =
  std::derived_from<std::decay_t<OStream>, sys::io::OutStream>;

#define DEF_OUTSTREAM_OP(T)                                                    \
    decltype(auto) operator<<(AOutStream auto &out, T value)                   \
    {                                                                          \
        write_repr(static_cast<OutStream &>(out), value);                      \
        return out;                                                            \
    }

inline StreamResult
write_repr(OutStream &out, bl::string_view str)
{
    auto [_, r] =
      write(out,
            bl::span<const bl::byte>{
              reinterpret_cast<const bl::byte *>(str.data()), str.size() });
    return r;
}

inline StreamResult
write_repr(OutStream &out, char c)
{
    return write_repr(out, bl::string_view{ &c, 1 });
}

#define DEF_OPAQUE_OUTSTREAM_OP(T)                                             \
    SYS_API StreamResult write_repr(OutStream &out, T x);                      \
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

SYS_API StreamResult
write_repr(OutStream &out, const void *x);

#undef DEF_OPAQUE_OUTSTREAM_OP
#undef DEF_OUTSTREAM_OP

template<std::same_as<bool> T>
decltype(auto)
operator<<(AOutStream auto &out, T x)
{
    using namespace bl::literals;
    return out << (x ? BL_STRING_VIEW("true") : BL_STRING_VIEW("false"));
}

decltype(auto)
operator<<(AOutStream auto &out, const bl::string &str)
{
    return out << bl::string_view{ str };
}

struct ptr
{
    const void *p;
};

decltype(auto)
operator<<(AOutStream auto &out, const ptr &p)
{
    write_repr(out, p.p);
    return out;
}

} // namespace sys::io

#endif
