#ifndef SYS_IO_STREAM_HPP
#define SYS_IO_STREAM_HPP

#include "pp/enum.hpp"
#include "sys/conf.hpp"
#include "sys/fiber.hpp"

#include <cstring>
#include <span>
#include <string>
#include <string_view>

#undef stdin
#undef stdout
#undef stderr
#undef EOF

namespace sys {
namespace io {

struct OutStream;
struct InStream;
struct IOStream;

HU_NODISCARD SYS_API InStream &
stdin();

HU_NODISCARD SYS_API OutStream &
stdout();

HU_NODISCARD SYS_API OutStream &
stderr();

#define SYS_STREAM_RESULT_ENUM_DEF(T, V0, V)                                   \
    T(StreamResult, uint8_t, V0(OK) V(Blocked) V(EOF) V(Closed) V(Error))

PP_DEF_ENUM_WITH_API(SYS_API, SYS_STREAM_RESULT_ENUM_DEF);

using StreamFlags = uint16_t;

inline constexpr StreamFlags SF_IN_EOF = 1;
inline constexpr StreamFlags SF_OUT_EOF = 2;
inline constexpr StreamFlags SF_IN_CLOSED = 4;
inline constexpr StreamFlags SF_OUT_CLOSED = 8;
inline constexpr StreamFlags SF_CLOSABLE = 16;
inline constexpr StreamFlags SF_LINEBUF = 32;

struct SYS_API InStream
{
    InStream();
    InStream(const InStream &) = default;
    InStream(InStream &&) = default;
    InStream &operator=(const InStream &) = default;
    InStream &operator=(InStream &&) = default;
    virtual ~InStream();

    void close();

    bool closed() const { return rflags() & SF_IN_CLOSED; }
    bool readable() const
    {
        return (rflags() & (SF_IN_CLOSED | SF_IN_EOF)) == 0;
    }
    bool closable() const { return rflags() & SF_CLOSABLE; }

    InStream &closable(bool yesno)
    {
        if (yesno)
            rflags() |= SF_CLOSABLE;
        else
            rflags() &= ~SF_CLOSABLE;
        return *this;
    }

    std::pair<size_t, StreamResult> read(std::span<char>);

protected:
    virtual StreamResult basic_read(size_t &s, char *buf) = 0;
    virtual StreamResult basic_close_in(bool flush_only) = 0;
    virtual const StreamFlags &rflags() const;
    virtual StreamFlags &rflags();

private:
    StreamFlags _rflags{ SF_CLOSABLE };
};

struct SYS_API OutStream
{
    OutStream();
    OutStream(const OutStream &) = default;
    OutStream(OutStream &&) = default;
    OutStream &operator=(const OutStream &) = default;
    OutStream &operator=(OutStream &&) = default;
    virtual ~OutStream();

    void close();

    bool closed() const { return wflags & SF_OUT_CLOSED; }

    bool writeable() const
    {
        return (wflags & (SF_OUT_CLOSED | SF_OUT_EOF)) == 0;
    }

    bool closable() const { return wflags & SF_CLOSABLE; }

    OutStream &closable(bool yesno)
    {
        if (yesno)
            wflags |= SF_CLOSABLE;
        else
            wflags &= ~SF_CLOSABLE;
        return *this;
    }

    bool line_buffered() const { return wflags & SF_LINEBUF; }
    OutStream &line_buffered(bool yesno)
    {
        if (yesno)
            wflags |= SF_LINEBUF;
        else
            wflags &= ~SF_LINEBUF;
        return *this;
    }

    std::pair<size_t, StreamResult> write(std::span<const char>);
    StreamResult flush();

protected:
    virtual StreamResult basic_write(size_t &s, const char *buf) = 0;
    virtual StreamResult basic_flush() = 0;
    virtual StreamResult basic_close_out() = 0;
    StreamFlags wflags{ SF_CLOSABLE };
};

struct SYS_API IOStream
  : public InStream
  , public OutStream
{
    IOStream();
    IOStream(const IOStream &) = default;
    IOStream(IOStream &&) = default;
    IOStream &operator=(const IOStream &) = default;
    IOStream &operator=(IOStream &&) = default;
    ~IOStream() override;

    using OutStream::close;
    using OutStream::closed;

    IOStream &closable(bool yesno)
    {
        OutStream::closable(yesno);
        return *this;
    }

protected:
    StreamResult basic_close_out() final;
    StreamResult basic_close_in(bool flush_only) final;

    virtual StreamResult basic_close() = 0;

    const StreamFlags &rflags() const final;
    StreamFlags &rflags() final;
};

struct SYS_API NullStream : public IOStream
{
    NullStream();

protected:
    StreamResult basic_flush() final;
    StreamResult basic_close() final;
    StreamResult basic_read(size_t &, char *) final;
    StreamResult basic_write(size_t &, const char *) final;
};

struct SYS_API ByteStream : public IOStream
{
    explicit ByteStream(size_t bufsize = 32);
    ByteStream(std::string_view);

    ~ByteStream() override;

    size_t size() const { return buffer.size(); }
    const char *data() const { return buffer.data(); }
    char *data() { return buffer.data(); }
    std::string str() const & { return std::string(data(), size()); }
    std::string str() const && { return std::move(buffer); }

    operator std::string_view() const { return { data(), size() }; }

    void truncate(size_t);

protected:
    StreamResult basic_flush() final override;
    StreamResult basic_close() final override;
    StreamResult basic_read(size_t &, char *) final override;
    StreamResult basic_write(size_t &, const char *) final override;

private:
    std::string buffer;
    size_t read_cursor;
};

struct SYS_API CooperativeInStream : public InStream
{
    InStream *in;
    Fiber *io_handler; // switched to on blocking reads
    Fiber *stream_user;

    CooperativeInStream(InStream *in, Fiber *ioh, Fiber *su);

protected:
    virtual StreamResult basic_close_in(bool flush_only) final override;
    virtual StreamResult basic_read(size_t &, char *) final override;
};

#define DEF_OUTSTREAM_OP(T)                                                    \
    template<typename OStream>                                                 \
    std::enable_if_t<std::is_base_of_v<sys::io::OutStream, OStream>, OStream>  \
      &operator<<(OStream &out, T value)                                       \
    {                                                                          \
        write_repr(static_cast<OutStream &>(out), value);                      \
        return out;                                                            \
    }

inline StreamResult
write_repr(OutStream &out, std::string_view str)
{
    if (out.writeable()) {
        auto [_, r] = out.write(str);
        return r;
    }
    return StreamResult::OK;
}

inline StreamResult
write_repr(OutStream &out, const char *str)
{
    if (!out.writeable() || !str)
        return StreamResult::OK;
    auto [_, r] = out.write({ str, strlen(str) });
    return r;
}

inline StreamResult
write_repr(OutStream &out, char c)
{
    return write_repr(out, std::string_view{ &c, 1 });
}

inline StreamResult
write_repr(OutStream &out, signed char c)
{
    return write_repr(out, static_cast<char>(c));
}

inline StreamResult
write_repr(OutStream &out, unsigned char c)
{
    return write_repr(out, static_cast<char>(c));
}

#define DEF_OPAQUE_OUTSTREAM_OP(T)                                             \
    SYS_API StreamResult write_repr(OutStream &out, T x);                      \
    DEF_OUTSTREAM_OP(T)

DEF_OUTSTREAM_OP(char);
DEF_OUTSTREAM_OP(signed char);
DEF_OUTSTREAM_OP(unsigned char);
DEF_OUTSTREAM_OP(const char *);
DEF_OUTSTREAM_OP(std::string_view);

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
DEF_OPAQUE_OUTSTREAM_OP(const void *);

#undef DEF_OPAQUE_OUTSTREAM_OP
#undef DEF_OUTSTREAM_OP

template<typename OStream>
std::enable_if_t<std::is_base_of_v<sys::io::OutStream, OStream>, OStream> &
operator<<(OStream &out, bool x)
{
    return out << (x ? "true" : "false");
}

template<typename OStream>
std::enable_if_t<std::is_base_of_v<sys::io::OutStream, OStream>, OStream> &
operator<<(OStream &out, const std::string &str)
{
    return out << std::string_view(str);
}

template<typename OStream>
std::enable_if_t<std::is_base_of_v<sys::io::OutStream, OStream>, OStream> &
operator<<(OStream &out, char *str)
{
    return out << static_cast<const char *>(str);
}

template<typename OStream, typename T>
std::enable_if_t<std::is_base_of_v<sys::io::OutStream, OStream>, OStream> &
operator<<(OStream &out, const T *ptr)
{
    return out << static_cast<const void *>(ptr);
}

} // namespace io
} // namespace sys
#endif
