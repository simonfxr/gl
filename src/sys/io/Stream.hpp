#ifndef SYS_IO_STREAM_HPP
#define SYS_IO_STREAM_HPP

#include "sys/io/Stream_fwd.hpp"

#include "bl/string.hpp"
#include "bl/string_view.hpp"
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

struct SYS_API InStream
{
    InStream();
    virtual ~InStream();

    void close();

    bool closed() const { return flags & SF_IN_CLOSED; }
    bool readable() const { return (flags & (SF_IN_CLOSED | SF_IN_EOF)) == 0; }
    bool closable() const { return flags & SF_CLOSABLE; }

    InStream &closable(bool yesno)
    {
        if (yesno)
            flags |= SF_CLOSABLE;
        else
            flags &= ~SF_CLOSABLE;
        return *this;
    }

    StreamResult read(size_t &s, char *buf);

protected:
    virtual StreamResult basic_read(size_t &s, char *buf) = 0;
    virtual StreamResult basic_close_in(bool flush_only) = 0;
    InStream(StreamFlags &);

    StreamFlags &flags;
    StreamFlags _flags_store;
};

struct SYS_API OutStream
{
    OutStream();
    virtual ~OutStream();

    void close();

    bool closed() const { return flags & SF_OUT_CLOSED; }

    bool writeable() const
    {
        return (flags & (SF_OUT_CLOSED | SF_OUT_EOF)) == 0;
    }

    bool closable() const { return flags & SF_CLOSABLE; }

    OutStream &closable(bool yesno)
    {
        if (yesno)
            flags |= SF_CLOSABLE;
        else
            flags &= ~SF_CLOSABLE;
        return *this;
    }

    StreamResult write(size_t &s, const char *buf);
    StreamResult flush();

protected:
    virtual StreamResult basic_write(size_t &s, const char *buf) = 0;
    virtual StreamResult basic_flush() = 0;
    virtual StreamResult basic_close_out() = 0;
    StreamFlags flags;
};

struct SYS_API IOStream
  : public InStream
  , public OutStream
{
    IOStream();
    virtual ~IOStream() override;

    using OutStream::close;
    using OutStream::closed;

    IOStream &closable(bool yesno)
    {
        OutStream::closable(yesno);
        return *this;
    }

protected:
    StreamResult basic_close_out() final override;
    StreamResult basic_close_in(bool flush_only) final override;

    virtual StreamResult basic_close() = 0;

    using OutStream::flags;
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
    bl::string str() const && { return bl::move(buffer); }

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
    Fiber *io_handler; // switched to on blocking reads
    Fiber *stream_user;

    CooperativeInStream(InStream *in, Fiber *ioh, Fiber *su);

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
    if (out.writeable()) {
        size_t s = str.size();
        return out.write(s, str.data());
    }
    return StreamResult::OK;
}

inline StreamResult
write_repr(OutStream &out, const char *str)
{
    if (!out.writeable() || !str)
        return StreamResult::OK;
    size_t s = strlen(str);
    return out.write(s, str);
}

inline StreamResult
write_repr(OutStream &out, char c)
{
    if (!out.writeable())
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
