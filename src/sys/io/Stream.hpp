#ifndef SYS_IO_STREAM_HPP
#define SYS_IO_STREAM_HPP

#include "sys/conf.hpp"
#include "sys/fiber.hpp"

#include <sstream>
#include <string>
#include <vector>

#ifdef SYSTEM_WINDOWS
#  undef stdout
#  undef stderr
#  define STDOUT_FILE (&__iob_func()[1])
#  define STDERR_FILE (&__iob_func()[2])
#else
#  define STDOUT_FILE ::stdout
#  define STDERR_FILE ::stderr
#endif

#define BASIC_STREAM_READ_STATE_IMPL \
    BasicStream& read_state() OVERRIDE { return *this; } \
    const BasicStream& read_state() const OVERRIDE { return *this; }

#define BASIC_STREAM_WRITE_STATE_IMPL \
    BasicStream& write_state() OVERRIDE { return *this;  } \
    const BasicStream& write_state() const OVERRIDE { return *this; }

#define BASIC_STREAM_STATE_IMPL \
    BASIC_STREAM_READ_STATE_IMPL \
    BASIC_STREAM_WRITE_STATE_IMPL

namespace sys {

namespace io {

using namespace defs;

struct OutStream;
struct InStream;
struct StreamEndl;

SYS_API OutStream& stdout();
SYS_API OutStream& stderr();

extern SYS_API const StreamEndl endl;

enum StreamResult {
    StreamOK,
    StreamBlocked,
    StreamEOF,
    StreamClosed,
    StreamError
};

namespace {

typedef uint32 StreamFlags;

const StreamFlags SS_IN_EOF = 1;
const StreamFlags SS_OUT_EOF = 2;
const StreamFlags SS_CLOSED = 4;
const StreamFlags SS_DONT_CLOSE = 8;

} // namespace anon

struct SYS_API BasicStream {
    StreamFlags _state = 0;

    void close() {
        close_flush();
        if ((_state & SS_CLOSED) == 0) {
            basic_close();
            _state |= SS_CLOSED;
        }
    }

    void dontClose(bool dont) {
        _state = dont ? _state | SS_DONT_CLOSE
                 : _state & ~SS_DONT_CLOSE;
    }

    bool flags(StreamFlags flags) const {
        return (_state & flags) == flags;
    }

    bool closed() const { return flags(SS_CLOSED); }
    
    bool inEOF() const { return flags(SS_IN_EOF);  }

    bool outEOF() const { return flags(SS_OUT_EOF); }

    bool writeable() const {
        return (_state & SS_CLOSED) == 0 && (_state & SS_OUT_EOF) == 0;
    }
    
protected:
    virtual void basic_close() = 0;
    virtual void close_flush() = 0;

    void destructor() {
        if ((_state & (SS_CLOSED | SS_DONT_CLOSE)) == 0) {
            basic_close();
            _state |= SS_CLOSED;
        }
    }
};

struct SYS_API InStream  {

    virtual ~InStream() {}

    virtual BasicStream& read_state() = 0;
    virtual const BasicStream& read_state() const = 0;

    StreamResult read(size& s, char *b) {
        StreamFlags& st = read_state()._state;

        if ((st & SS_CLOSED) != 0) {
            s = 0;
            return StreamClosed;
        }

        if ((st & SS_IN_EOF) != 0) {
            s = 0;
            return StreamEOF;
        }

        StreamResult r = basic_read(s, b);
        if (r == StreamEOF)
            st |= SS_IN_EOF;

        return r;
    }

protected:

    virtual StreamResult basic_read(size&, char *) = 0;
};

struct SYS_API OutStream  {

    virtual ~OutStream() {}

    virtual BasicStream& write_state() = 0;
    virtual const BasicStream& write_state() const = 0;

    StreamResult write(size& s, const char *b) {
        StreamFlags& st = write_state()._state;
        
        if ((st & SS_CLOSED) != 0) {
            s = 0;
            return StreamClosed;
        }
        
        if ((st & SS_OUT_EOF) != 0) {
            s = 0;
            return StreamEOF;
        }
        
        StreamResult r = basic_write(s, b);
        if (r == StreamEOF)
            st |= SS_OUT_EOF;
        
        return r;
    }

    StreamResult flush() {
        StreamFlags& st = write_state()._state;

        if ((st & SS_CLOSED) != 0)
            return StreamClosed;

        if ((st & SS_OUT_EOF) != 0)
            return StreamEOF;
        
        StreamResult r = basic_flush();
        if (r == StreamEOF)
            st |= SS_OUT_EOF;

        return r;
    }

protected:

    virtual StreamResult basic_write(size&, const char *) = 0;
    virtual StreamResult basic_flush() { return StreamOK; }
};

struct SYS_API IOStream : public InStream, public OutStream {
    virtual ~IOStream() {}
};

struct SYS_API AbstractIOStream : public BasicStream, public IOStream {
    virtual ~AbstractIOStream() {}
    BASIC_STREAM_STATE_IMPL;

    void close_flush() OVERRIDE { basic_flush(); }
};

struct SYS_API AbstractOutStream : public BasicStream, public OutStream {
    virtual ~AbstractOutStream() {}
    BASIC_STREAM_WRITE_STATE_IMPL;

    void close_flush() OVERRIDE { basic_flush(); }
};

struct SYS_API AbstractInStream : public BasicStream, public InStream {
    virtual ~AbstractInStream() {}
    BASIC_STREAM_READ_STATE_IMPL;

    void close_flush() OVERRIDE {}
};

struct SYS_API StdOutStream : public AbstractOutStream {
    std::ostream *_out;
    StdOutStream(std::ostream&);
    ~StdOutStream() { destructor(); }
    void out(std::ostream& o) { _out = &o; }
protected:
    virtual StreamResult basic_write(size&, const char *) FINAL OVERRIDE;
    virtual StreamResult basic_flush() FINAL OVERRIDE;
    virtual void basic_close() FINAL OVERRIDE;
private:
    StdOutStream(const StdOutStream&);
    StdOutStream& operator =(const StdOutStream&);
};

struct SYS_API StdInStream : public AbstractInStream {
    std::istream *_in;
    StdInStream(std::istream&);
    ~StdInStream() { destructor(); }
protected:
    virtual StreamResult basic_read(size&, char *) FINAL OVERRIDE;
    virtual void basic_close() FINAL OVERRIDE;
private:
    StdInStream(const StdInStream&);
    StdInStream& operator =(const StdInStream&);
};

struct SYS_API FileStream : public AbstractIOStream {
    struct FILE; // use as dummy type, avoid importing stdio.h, on
                 // windows stdin/stdout/stderr are macros, which
                 // clashes with our definitions
    FILE *_file;
    FileStream(FILE *file = nullptr);
    FileStream(const std::string& path, const std::string& mode);
    ~FileStream() { destructor(); }
    bool isOpen() const { return _file != 0; }
    bool open(const std::string& path, const std::string& mode);
protected:
    virtual StreamResult basic_read(size&, char *) FINAL OVERRIDE;
    virtual StreamResult basic_write(size&, const char *) FINAL OVERRIDE;
    virtual void basic_close() FINAL OVERRIDE;
    virtual StreamResult basic_flush() FINAL OVERRIDE;
private:
    FileStream(const FileStream&);
    FileStream& operator =(const FileStream&);
};

struct SYS_API NullStream : public AbstractIOStream {
    NullStream() { close(); }
protected:
    void basic_close() FINAL OVERRIDE;
    StreamResult basic_read(size&, char *) FINAL OVERRIDE;
    StreamResult basic_write(size&, const char *) FINAL OVERRIDE;
};

struct SYS_API CooperativeInStream : public AbstractInStream {
    InStream *in;
    Fiber *io_handler; // switched to on blocking reads
    Fiber *stream_user;

    CooperativeInStream(InStream *in, Fiber *io_handler, Fiber *stream_user);
protected:
    virtual void basic_close() FINAL OVERRIDE;
    virtual StreamResult basic_read(size&, char *) FINAL OVERRIDE;
};

SYS_API OutStream& operator <<(OutStream& out, const std::string& str);

SYS_API OutStream& operator <<(OutStream& out, const char *str);

SYS_API OutStream& operator <<(OutStream& out, const StreamEndl&);

SYS_API OutStream& operator <<(OutStream& out, std::ostringstream&);

SYS_API OutStream& operator <<(OutStream& out, std::stringstream&);

template <typename T>
OutStream& operator <<(OutStream& out, const T& x) {
    if (out.write_state().writeable()) {
        std::ostringstream s;
        s << x;
        out << s;
    }
    return out;
}

} // namespace io

} // namespace sys

#endif
