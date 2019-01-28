#ifndef SYS_IO_STREAM_HPP
#define SYS_IO_STREAM_HPP

#include "sys/io/Stream_fwd.hpp"

#include "bl/non_copyable.hpp"
#include "bl/string.hpp"
#include "bl/string_view.hpp"
#include "bl/type_traits.hpp"
#include "sys/fiber.hpp"

#undef EOF
#undef stdin
#undef stderr
#undef stdout

namespace sys {
namespace io {

struct OutStream;
struct InStream;
struct StreamEndl;
struct IOStream;

PP_DEF_ENUM_WITH_API(SYS_API, SYS_STREAM_RESULT_ENUM_DEF);

struct StreamState
{
    constexpr bool closable() const { return flags() & SF_CLOSABLE; }

    constexpr StreamState &closable(bool yesno)
    {
        if (yesno)
            flags() |= SF_CLOSABLE;
        else
            flags() &= ~SF_CLOSABLE;
        return *this;
    }

    constexpr StreamFlags flags() const { return _flags; }

protected:
    constexpr StreamFlags &flags() { return _flags; }

private:
    StreamFlags _flags{};
};

struct SYS_API InStream : virtual StreamState
{
    InStream();
    virtual ~InStream();

    InStream(const InStream &) = default;
    InStream(InStream &&) = default;

    InStream &operator=(const InStream &) = default;
    InStream &operator=(InStream &&) = default;

    void close();

    StreamResult read(size_t &s, char *buf);

    bool readable() const
    {
        return (flags() & (SF_IN_CLOSED | SF_IN_EOF)) == 0;
    }

    bool closed() const { return flags() & SF_IN_CLOSED; }

protected:
    virtual StreamResult basic_read(size_t &s, char *buf) = 0;
    virtual StreamResult basic_close_in(bool flush_only) = 0;
    InStream(StreamFlags &);
};

struct SYS_API OutStream : StreamState
{
    OutStream();
    virtual ~OutStream();

    OutStream(const OutStream &) = default;
    OutStream(OutStream &&) = default;

    OutStream &operator=(const OutStream &) = default;
    OutStream &operator=(OutStream &&) = default;

    bool closed() const { return flags() & SF_IN_CLOSED; }

    constexpr bool writable() const
    {
        return (flags() & (SF_OUT_CLOSED | SF_OUT_EOF)) == 0;
    }

    void close();

    StreamResult write(size_t &s, const char *buf);
    StreamResult flush();

protected:
    virtual StreamResult basic_write(size_t &s, const char *buf) = 0;
    virtual StreamResult basic_flush() = 0;
    virtual StreamResult basic_close_out() = 0;
};

struct SYS_API IOStream
  : public InStream
  , public OutStream
{
    IOStream();
    virtual ~IOStream() override;

    IOStream(const IOStream &) = default;
    IOStream(IOStream &&) = default;

    IOStream &operator=(const IOStream &) = default;
    IOStream &operator=(IOStream &&) = default;

    using OutStream::close;

    StreamFlags flags() const { return OutStream::flags(); }

    IOStream &closable(bool yesno)
    {
        OutStream::closable(yesno);
        return *this;
    }

protected:
    StreamResult basic_close_out() final override;
    StreamResult basic_close_in(bool flush_only) final override;

    StreamFlags &flags() { return OutStream::flags(); }

    virtual StreamResult basic_close() = 0;
};

struct SYS_API NullStream : public IOStream
{
    NullStream();

protected:
    StreamResult basic_flush() final override;
    StreamResult basic_close() final override;
    StreamResult basic_read(size_t &, char *) final override;
    StreamResult basic_write(size_t &, const char *) final override;
};

struct SYS_API ByteStream : public IOStream
{
    explicit ByteStream(size_t bufsize = 32);
    ByteStream(bl::string_view);

    ~ByteStream() override;

    size_t size() const { return buffer.size(); }
    const char *data() const { return buffer.data(); }
    char *data() { return buffer.data(); }
    bl::string str() const & { return bl::string(data(), size()); }
    bl::string str() const && { return std::move(buffer); }

    operator bl::string_view() const { return { data(), size() }; }

    void truncate(size_t);

protected:
    StreamResult basic_flush() final override;
    StreamResult basic_close() final override;
    StreamResult basic_read(size_t &, char *) final override;
    StreamResult basic_write(size_t &, const char *) final override;

private:
    bl::string buffer;
    size_t read_cursor;
};

struct SYS_API CooperativeInStream : public InStream
{
    InStream *in;
    Fiber &io_handler; // switched to on blocking reads
    Fiber &stream_user;

    CooperativeInStream(InStream *in, Fiber &ioh, Fiber &su);

protected:
    virtual StreamResult basic_close_in(bool flush_only) final override;
    virtual StreamResult basic_read(size_t &, char *) final override;
};

#define DEF_OUTSTREAM_OP(T)                                                    \
    template<typename OStream>                                                 \
    bl::enable_if_t<std::is_base_of_v<sys::io::OutStream, OStream>, OStream>   \
      &operator<<(OStream &out, T value)                                       \
    {                                                                          \
        write_repr(static_cast<OutStream &>(out), value);                      \
        return out;                                                            \
    }

inline StreamResult
write_repr(OutStream &out, bl::string_view str)
{
    if (out.writable()) {
        size_t s = str.size();
        return out.write(s, str.data());
    }
    return StreamResult::OK;
}

inline StreamResult
write_repr(OutStream &out, const char *str)
{
    return write_repr(out, bl::string_view(str));
}

inline StreamResult
write_repr(OutStream &out, char c)
{
    if (!out.writable())
        return StreamResult::OK;
    size_t s = 1;
    return out.write(s, &c);
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

inline StreamResult
write_repr(OutStream &out, const StreamEndl & /*unused*/)
{
    auto ret1 = write_repr(out, '\n');
    auto ret2 = out.flush();
    return ret1 == StreamResult::OK ? ret2 : ret1;
}

DEF_OUTSTREAM_OP(char);
DEF_OUTSTREAM_OP(signed char);
DEF_OUTSTREAM_OP(unsigned char);
DEF_OUTSTREAM_OP(const char *);
DEF_OUTSTREAM_OP(bl::string_view);
DEF_OUTSTREAM_OP(const StreamEndl &);

DEF_OUTSTREAM_OP(short)
DEF_OUTSTREAM_OP(unsigned short)
DEF_OUTSTREAM_OP(int)
DEF_OUTSTREAM_OP(unsigned)
DEF_OUTSTREAM_OP(long)
DEF_OUTSTREAM_OP(unsigned long)
DEF_OUTSTREAM_OP(long long)
DEF_OUTSTREAM_OP(unsigned long long)

DEF_OUTSTREAM_OP(float);
DEF_OUTSTREAM_OP(double);
DEF_OUTSTREAM_OP(long double);
DEF_OUTSTREAM_OP(const void *);

#undef DEF_OUTSTREAM_OP

template<typename OStream>
inline bl::enable_if_t<std::is_base_of_v<sys::io::OutStream, OStream>, OStream>
  &
  operator<<(OStream &out, bool x)
{
    return out << (x ? "true" : "false");
}

template<typename OStream>
inline bl::enable_if_t<std::is_base_of_v<sys::io::OutStream, OStream>, OStream>
  &
  operator<<(OStream &out, const bl::string &str)
{
    return out << static_cast<bl::string_view>(str);
}

template<typename OStream>
inline bl::enable_if_t<std::is_base_of_v<sys::io::OutStream, OStream>, OStream>
  &
  operator<<(OStream &out, char *str)
{
    return out << static_cast<const char *>(str);
}

template<typename OStream, typename T>
inline bl::enable_if_t<std::is_base_of_v<sys::io::OutStream, OStream>, OStream>
  &
  operator<<(OStream &out, const T *ptr)
{
    return out << static_cast<const void *>(ptr);
}

} // namespace io
} // namespace sys
#endif
