#ifndef SYS_IO_STREAM_HPP
#define SYS_IO_STREAM_HPP

#include "defs.hpp"

#include <sstream>
#include <stdio.h>

namespace sys {

namespace io {

using namespace defs;

struct OutStream;
struct InStream;

OutStream& stdout();
OutStream& stderr();

OutStream& endl(OutStream&);

enum StreamResult {
    StreamOK,
    StreamBlocked,
    StreamEOF,
    StreamClosed,
    StreamError
};

namespace {

typedef uint32 StreamState;

const StreamState SS_IN_EOF = 1;
const StreamState SS_OUT_EOF = 2;
const StreamState SS_CLOSED = 4;
const StreamState SS_DONT_CLOSE = 8;

} // namespace anon

struct StreamStateImpl {
private:
    StreamState _state;
public:
    StreamStateImpl() : _state(0) {}
    StreamState state() const { return _state; }
    bool dontClose() const { return (_state & SS_DONT_CLOSE) != 0; }
    void dontClose(bool dont) {
        _state = dont
            ? _state | SS_DONT_CLOSE
            : _state & ~SS_DONT_CLOSE;
    }
protected:
    void state(StreamState s) { _state = s; }
};

struct BasicStream {
    virtual ~BasicStream() {}

    void close() {
        StreamState s = basic_state();
        if ((s & SS_CLOSED) == 0) {
            basic_state(s | SS_CLOSED);
            basic_close();
        }
    }

    
protected:
    virtual StreamState basic_state() const = 0;
    virtual void basic_state(StreamState) = 0;
    virtual void basic_close() {}

    void destructor() {
        StreamState st = basic_state();
        if ((st & SS_CLOSED) == 0 &&
            (st & SS_DONT_CLOSE) == 0) {
            basic_state(st | SS_CLOSED);
            basic_close();
        }        
    }
};

struct InStream : public BasicStream {
    virtual ~InStream() {}
    
    StreamResult read(size& s, char *b) {
        StreamState st = this->basic_state();
        
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
            this->basic_state(this->basic_state() | SS_IN_EOF);
        
        return r;
    }
    
protected:
    virtual StreamResult basic_read(size&, char *) = 0;
};

struct OutStream : public BasicStream {
    virtual ~OutStream() {}

    StreamResult write(size& s, const char *b) {
        StreamState st = this->basic_state();
        
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
            this->basic_state(this->basic_state() | SS_OUT_EOF);
        
        return r;
    }

    void flush() { basic_flush(); }

protected:
    virtual StreamResult basic_write(size&, const char *) = 0;
    virtual void basic_flush() {}
};

struct AbstractInStream : public InStream, public StreamStateImpl {
    virtual ~AbstractInStream() {
        this->destructor();
    }
protected:
    StreamState basic_state() const {
        return this->state();
    }
    
    void basic_state(StreamState s) {
        this->state(s);
    }
};

struct AbstractOutStream : public OutStream, public StreamStateImpl {
    virtual ~AbstractOutStream() {
        this->destructor();
    }
protected:
    StreamState basic_state() const {
        return this->state();
    }
    
    void basic_state(StreamState s) {
        this->state(s);
    }
};

struct IOStream : public InStream, public OutStream, public StreamStateImpl {
    virtual ~IOStream() {
        InStream::destructor();
    }
protected:
    StreamState basic_state() const {
        return this->state();
    }
    
    void basic_state(StreamState s) {
        this->state(s);
    }

    virtual void basic_close() {
        InStream::basic_close();
    }

};

struct StdOutStream : public AbstractOutStream {
    std::ostream& out;
    StdOutStream(std::ostream&);
    
protected:
    StreamResult basic_write(size& s, const char *b);
    void basic_flush();
};

struct FileStream : public IOStream {
    FILE *file;
    FileStream(FILE *);

protected:
    StreamResult basic_read(size&, char *);
    StreamResult basic_write(size&, const char *);
    void basic_close();
    void basic_flush();
};

template <typename T>
OutStream& operator <<(OutStream& out, const T x) {
    std::ostringstream s;
    s << x;
    const std::string str = s.str();
    size n = str.size();
    out.write(n, str.data());
    return out;
}

inline OutStream& operator <<(OutStream& out, OutStream& (*func)(OutStream&)) {
    return func(out);
}

} // namespace io

} // namespace sys

#endif
