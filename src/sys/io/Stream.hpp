#ifndef SYS_IO_STREAM_HPP
#define SYS_IO_STREAM_HPP

#include "sys/conf.hpp"
#include "sys/fiber.hpp"

#include <string>
#include <string_view>
#include <vector>

#ifdef HU_OS_WINDOWS
#define STDIN_FILE __acrt_iob_func(0)
#define STDOUT_FILE __acrt_iob_func(1)
#define STDERR_FILE __acrt_iob_func(2)
#else
#define STDIN_FILE ::stdin
#define STDOUT_FILE ::stdout
#define STDERR_FILE ::stderr
#endif

#undef stdin
#undef stdout
#undef stderr
#undef EOF

namespace sys {

namespace io {

struct OutStream;
struct InStream;
struct StreamEndl;
struct IOStream;

SYS_API InStream&
stdin();

SYS_API OutStream &
stdout();

SYS_API OutStream &
stderr();

extern SYS_API const StreamEndl endl;

enum class StreamResult : uint16_t
{
    OK,
    Blocked,
    EOF,
    Closed,
    Error
};

using StreamFlags = uint16_t;

namespace {

inline constexpr StreamFlags SF_IN_EOF = 1;
inline constexpr StreamFlags SF_OUT_EOF = 2;
inline constexpr StreamFlags SF_IN_CLOSED = 4;
inline constexpr StreamFlags SF_OUT_CLOSED = 8;
inline constexpr StreamFlags SF_CLOSABLE = 16;

} // namespace

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

#if 0
struct SYS_API FileStream : public IOStream
{
    struct FILE; // use as dummy type, avoid importing stdio.h, on
                 // windows stdin/stdout/stderr are macros, which
                 // clashes with our definitions
    FILE *_file;
    FileStream(FILE *file = nullptr);
    FileStream(const std::string &path, const std::string &mode);
    ~FileStream() override;
    bool isOpen() const { return _file != nullptr; }
    bool open(const std::string &path, const std::string &mode);

protected:
    virtual StreamResult basic_read(size_t &, char *) final override;
    virtual StreamResult basic_write(size_t &, const char *) final override;
    virtual StreamResult basic_close() final override;
    virtual StreamResult basic_flush() final override;

    FileStream(const FileStream &) = delete;
    FileStream &operator=(const FileStream &) = delete;
};
#endif
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

    ByteStream(size_t bufsize = 32);
    ByteStream(const char *buf, size_t sz);
    ByteStream(const std::string &);

    ~ByteStream() override;

    size_t size() const { return buffer.size(); }
    const char *data() const { return buffer.data(); }
    char *data() { return buffer.data(); }
    std::string str() const { return std::string(data(), size()); }

    void truncate(size_t);

protected:
    StreamResult basic_flush() final override;
    StreamResult basic_close() final override;
    StreamResult basic_read(size_t &, char *) final override;
    StreamResult basic_write(size_t &, const char *) final override;

private:
    std::vector<char> buffer;
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

SYS_API OutStream &
operator<<(OutStream &out, const std::string &str);

SYS_API OutStream &
operator<<(OutStream &out, std::string_view str);

SYS_API OutStream &
operator<<(OutStream &out, const char *str);

SYS_API OutStream &
operator<<(OutStream &out, char c);

inline OutStream &
operator<<(OutStream &out, char *str)
{
    const char *msg = str;
    return out << msg;
}

SYS_API OutStream &
operator<<(OutStream &out, const void *ptr);

inline OutStream &
operator<<(OutStream &out, void *ptr)
{
    const void *p = ptr;
    return out << p;
}

SYS_API OutStream &
operator<<(OutStream &out, const StreamEndl &);

template<typename T>
OutStream &
operator<<(OutStream &out, const T &x)
{
    if (out.writeable())
        out << std::to_string(x);
    return out;
}

} // namespace io

} // namespace sys

#endif
