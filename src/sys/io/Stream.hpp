#ifndef SYS_IO_STREAM_HPP
#define SYS_IO_STREAM_HPP

#include "sys/conf.hpp"
#include "sys/fiber.hpp"

#include <string>
#include <vector>

#ifdef _STDIO_H
#undef stdin
#undef stdout
#undef stderr
#undef EOF
// #  ifndef SYS_IO_STREAM_NOWARN
#if 0
#warning "<stdio.h> and sys/io/Stream.hpp included at the same time"
#endif
#endif

#ifdef SYSTEM_WINDOWS
#define STDOUT_FILE (&__iob_func()[1])
#define STDERR_FILE (&__iob_func()[2])
#else
#define STDOUT_FILE ::stdout
#define STDERR_FILE ::stderr
#endif

namespace sys {

namespace io {

using namespace defs;

struct OutStream;
struct InStream;
struct StreamEndl;
struct IOStream;

SYS_API OutStream &
stdout();
SYS_API OutStream &
stderr();

extern SYS_API const StreamEndl endl;

enum class StreamResult : uint16
{
    OK,
    Blocked,
    EOF,
    Closed,
    Error
};

typedef uint16 StreamFlags;

namespace {

const StreamFlags SF_IN_EOF = 1;
const StreamFlags SF_OUT_EOF = 2;
const StreamFlags SF_IN_CLOSED = 4;
const StreamFlags SF_OUT_CLOSED = 8;
const StreamFlags SF_CLOSABLE = 16;

} // namespace

struct SYS_API InStream
{
    InStream();
    virtual ~InStream();

    void close();

    bool closed() const { return *_flags & SF_IN_CLOSED; }
    bool readable() const
    {
        return (*_flags & (SF_IN_CLOSED | SF_IN_EOF)) == 0;
    }
    bool closable() const { return *_flags & SF_CLOSABLE; }

    InStream &closable(bool yesno)
    {
        if (yesno)
            *_flags |= SF_CLOSABLE;
        else
            *_flags &= ~SF_CLOSABLE;
        return *this;
    }

    StreamResult read(size &s, char *buf);

protected:
    virtual StreamResult basic_read(size &s, char *buf) = 0;
    virtual StreamResult basic_close_in(bool flush_only) = 0;

    void init(StreamFlags *);

    StreamFlags *_flags;
    union
    {
        StreamFlags _flags_store;
        void *__pad{};
    };
};

struct SYS_API OutStream
{
    OutStream();
    virtual ~OutStream();

    void close();

    bool closed() const { return _flags & SF_OUT_CLOSED; }
    bool writeable() const
    {
        return (_flags & (SF_OUT_CLOSED | SF_OUT_EOF)) == 0;
    }
    bool closable() const { return _flags & SF_CLOSABLE; }

    OutStream &closable(bool yesno)
    {
        if (yesno)
            _flags |= SF_CLOSABLE;
        else
            _flags &= ~SF_CLOSABLE;
        return *this;
    }

    StreamResult write(size &s, const char *buf);
    StreamResult flush();

protected:
    virtual StreamResult basic_write(size &s, const char *buf) = 0;
    virtual StreamResult basic_flush() = 0;
    virtual StreamResult basic_close_out() = 0;

    void init(StreamFlags *);

    union
    {
        StreamFlags _flags;
        void *__pad{};
    };
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

    using OutStream::_flags;
};

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
    virtual StreamResult basic_read(size &, char *) final override;
    virtual StreamResult basic_write(size &, const char *) final override;
    virtual StreamResult basic_close() final override;
    virtual StreamResult basic_flush() final override;

    FileStream(const FileStream &) = delete;
    FileStream &operator=(const FileStream &) = delete;
};

struct SYS_API NullStream : public IOStream
{
    NullStream();

protected:
    StreamResult basic_flush() final override;
    StreamResult basic_close() final override;
    StreamResult basic_read(size &, char *) final override;
    StreamResult basic_write(size &, const char *) final override;
};

struct SYS_API CooperativeInStream : public InStream
{
    InStream *in;
    Fiber *io_handler; // switched to on blocking reads
    Fiber *stream_user;

    CooperativeInStream(InStream *in, Fiber *ioh, Fiber *su);

protected:
    virtual StreamResult basic_close_in(bool flush_only) final override;
    virtual StreamResult basic_read(size &, char *) final override;
};

struct SYS_API ByteStream : public IOStream
{

    std::vector<char> _buffer;
    defs::index _read_cursor;

    ByteStream(defs::size bufsize = 32);
    ByteStream(const char *buf, defs::size sz);
    ByteStream(const std::string &);

    ~ByteStream() override;

    defs::size size() const { return SIZE(_buffer.size()); }
    const char *data() const { return &_buffer.front(); }
    char *data() { return &_buffer.front(); }
    std::string str() const { return std::string(data(), UNSIZE(size())); }

    void truncate(defs::size);

protected:
    StreamResult basic_flush() final override;
    StreamResult basic_close() final override;
    StreamResult basic_read(defs::size &, char *) final override;
    StreamResult basic_write(defs::size &, const char *) final override;
};

SYS_API OutStream &
operator<<(OutStream &out, const std::string &str);

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
