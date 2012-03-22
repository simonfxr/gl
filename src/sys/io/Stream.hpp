#ifndef SYS_IO_STREAM_HPP
#define SYS_IO_STREAM_HPP

#include "sys/conf.hpp"

#include <sstream>
#include <string>
#include <stdio.h>
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

struct SYS_API BasicStreamState {
protected:
    virtual StreamFlags& basic_state() = 0;
    virtual StreamFlags basic_state() const = 0;
};

struct SYS_API BasicStream : public virtual BasicStreamState {
    void close() {
        close_flush();
        StreamFlags& s = basic_state();
        if ((s & SS_CLOSED) == 0) {
            basic_close();
            s |= SS_CLOSED;
        }
    }

    void dontClose(bool dont) {
        StreamFlags& s = basic_state();
        s = dont ? s | SS_DONT_CLOSE
                 : s & ~SS_DONT_CLOSE;
    }

    bool flags(StreamFlags flags) const {
        return (basic_state() & flags) == flags;
    }

    bool closed() const { return flags(SS_CLOSED); }
    
    bool inEOF() const { return flags(SS_IN_EOF);  }

    bool outEOF() const { return flags(SS_OUT_EOF); }

    bool writeable() const {
        StreamFlags s = basic_state();
        return (s & SS_CLOSED) == 0 && (s & SS_OUT_EOF) == 0;
    }
    
protected:
    virtual void basic_close() {}
    virtual void close_flush() {}

    void destructor() {
        StreamFlags& st = basic_state();
        if ((st & SS_CLOSED) == 0 &&
            (st & SS_DONT_CLOSE) == 0) {
            basic_close();
            st |= SS_CLOSED;
        }        
    }
};

struct SYS_API BasicInStream : public virtual BasicStreamState {
    StreamResult read(size& s, char *b) {
        StreamFlags& st = basic_state();
        
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

struct SYS_API BasicOutStream : public virtual BasicStreamState {
    StreamResult write(size& s, const char *b) {
        StreamFlags& st = basic_state();
        
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
        StreamFlags& st = basic_state();

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

struct SYS_API StreamState {
private:
    StreamFlags _state;
public:
    StreamState() : _state(0) {}
    StreamFlags state() const { return _state; }
    
protected:
    StreamFlags& state() { return _state; }
};

struct SYS_API InStream : public virtual BasicStream, public BasicInStream {
    virtual ~InStream() {}
};

struct SYS_API OutStream : public virtual BasicStream, public BasicOutStream {
    virtual ~OutStream() {}
protected:
    void close_flush() { flush(); }
};

struct SYS_API IOStream : public InStream, public OutStream {
    virtual ~IOStream() {}
};

struct SYS_API AbstractIOStream : public StreamState, public IOStream {
    virtual ~AbstractIOStream() {}
protected:
    StreamFlags basic_state() const { return state(); }
    StreamFlags& basic_state() { return state(); }  
};

struct SYS_API AbstractOutStream : public StreamState, public OutStream {
    virtual ~AbstractOutStream() {}
protected:
    StreamFlags basic_state() const { return state(); }
    StreamFlags& basic_state() { return state(); }
};

struct SYS_API AbstractInStream : public StreamState, public InStream {
    virtual ~AbstractInStream() {}
protected:
    StreamFlags basic_state() const { return state(); }
    StreamFlags& basic_state() { return state(); }
};

struct SYS_API StdOutStream : public AbstractOutStream {
    std::ostream *_out;
    StdOutStream(std::ostream&);
    ~StdOutStream() { destructor(); }
    void out(std::ostream& o) { _out = &o; }
protected:
    StreamResult basic_write(size&, const char *);
    StreamResult basic_flush();
};

struct SYS_API StdInStream : public AbstractInStream {
    std::istream *_in;
    StdInStream(std::istream&);
    ~StdInStream() { destructor(); }
protected:
    StreamResult basic_read(size&, char *);
};

struct SYS_API FileStream : public AbstractIOStream {
    FILE *file;
    FileStream(FILE *file = 0);
    FileStream(const std::string& path, const std::string& mode);
    ~FileStream() { destructor(); }

    bool isOpen() const { return file != 0; }
    bool open(const std::string& path, const std::string& mode);
    
protected:
    StreamResult basic_read(size&, char *);
    StreamResult basic_write(size&, const char *);
    void basic_close();
    StreamResult basic_flush();
};

struct SYS_API NullStream : public AbstractIOStream {
    NullStream() { close(); }
protected:
    StreamResult basic_read(size&, char *);
    StreamResult basic_write(size&, const char *);
};

struct SYS_API RecordingStream : public AbstractInStream {
    enum State {
        Recording,
        Replaying
    };
    
    InStream *in;
    std::vector<char> recorded;
    State state;
    defs::index cursor;

    RecordingStream(InStream& in);

    void replay();
    void record();
    void reset();
    void clear();

protected:
    void basic_close();
    StreamResult basic_read(size&, char *);
};

SYS_API OutStream& operator <<(OutStream& out, const std::string& str);

SYS_API OutStream& operator <<(OutStream& out, const char *str);

SYS_API OutStream& operator <<(OutStream& out, const StreamEndl&);

SYS_API OutStream& operator <<(OutStream& out, std::ostringstream&);

SYS_API OutStream& operator <<(OutStream& out, std::stringstream&);

template <typename T>
OutStream& operator <<(OutStream& out, const T& x) {
    if (out.writeable()) {
        std::ostringstream s;
        s << x;
        out << s;
    }
    return out;
}

} // namespace io

} // namespace sys

#endif
