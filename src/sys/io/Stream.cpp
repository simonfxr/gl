#define SYS_IO_STREAM_CPP 1

#include "err/err.hpp"
#include "sys/module.hpp"

#include <cstring>
#include <cstdio>

#ifdef SYSTEM_UNIX
#  include <errno.h>
#endif

#include "sys/io/Stream.hpp"

namespace sys {

namespace io {

using defs::index;

namespace {

FILE *castFILE(FileStream::FILE *p) {
    return reinterpret_cast<FILE *>(p);
}

FileStream::FILE *castFILE(FILE *p) {
    return reinterpret_cast<FileStream::FILE* >(p);
}

struct StreamState {
    static StreamResult track(StreamFlags eof, StreamFlags *, StreamResult r);
};

StreamResult StreamState::track(StreamFlags eof, StreamFlags *flags, StreamResult res) {
    switch (res) {
    case StreamResult::OK: return res;
    case StreamResult::Blocked: return res;
    case StreamResult::EOF: *flags |= eof; return res;
    case StreamResult::Closed: return res;
    case StreamResult::Error: return res;
    }
    ASSERT_FAIL();
}

} // namespace anon

InStream::InStream() : _flags(&_flags_store), _flags_store(SF_CLOSABLE) {}

InStream::~InStream() {}

void InStream::close() {
    if (closed())
        return;
    
    StreamResult ret;
    if (closable()) {
        *_flags |= SF_IN_CLOSED;
        ret = basic_close_in(false);
    } else {
        ret = basic_close_in(true);
    }
    StreamState::track(SF_IN_EOF | SF_OUT_EOF, _flags, ret);
}

StreamResult InStream::read(size& s, char *buf) {
    if (!readable()) {
        s = 0;
        return closed() ? StreamResult::Closed : StreamResult::EOF;
    } else if (s <= 0) {
        s = 0;
        return StreamResult::OK;
    }
    return StreamState::track(SF_IN_EOF, _flags, basic_read(s, buf));
}

void InStream::init(StreamFlags *flags) {
    _flags = flags;
}

OutStream::OutStream() : _flags(SF_CLOSABLE) {}

OutStream::~OutStream() {}

void OutStream::close() {
    if (closed())
        return;

    StreamResult ret;
    if (closable()) {
        _flags |= SF_OUT_CLOSED;
        ret = basic_close_out();
    } else {
        ret = basic_flush();
    }
    StreamState::track(SF_IN_EOF | SF_OUT_EOF, &_flags, ret);
}

StreamResult OutStream::write(size& s, const char *buf) {
    if (!writeable()) {
        s = 0;
        return closed() ? StreamResult::Closed : StreamResult::EOF;
    } else if (s <= 0) {
        s = 0;
        return StreamResult::OK;
    }
    return StreamState::track(SF_OUT_EOF, &_flags, basic_write(s, buf));
}

StreamResult OutStream::flush() {
    if (closed())
        return StreamResult::Closed;
    return StreamState::track(SF_OUT_EOF, &_flags, basic_flush());
}

IOStream::IOStream() {
    InStream::init(&this->_flags);
}

IOStream::~IOStream() {}

StreamResult IOStream::basic_close_in(bool flush_only) {
    if (flush_only) {
        return basic_flush();
    } else {
        _flags |= SF_OUT_CLOSED;
        return basic_close();
    }
}

StreamResult IOStream::basic_close_out() {
    _flags |= SF_IN_CLOSED;
    return basic_close();
}

Streams::Streams() :
    stdout(castFILE(STDOUT_FILE)),
    stderr(castFILE(STDERR_FILE))
{
    stdout.closable(false);
    stderr.closable(false);
}

OutStream& stdout() {
    return module->io_streams.stdout;
}

OutStream& stderr() {
    return module->io_streams.stderr;
}

struct StreamEndl {};
const StreamEndl endl = {};

OutStream& operator <<(OutStream& out, const std::string& str) {
    if (out.writeable()) {
        size s = SIZE(str.size());
        out.write(s, str.data());
    }
    return out;
}

OutStream& operator <<(OutStream& out, const char *str) {
    if (!out.writeable())
        return out;
    if (!str)
        return out;
    size s = SIZE(strlen(str));
    out.write(s, str);
    return out;
}

OutStream& operator <<(OutStream& out, char c) {
    if (!out.writeable())
        return out;
    size s = 1;
    out.write(s, &c);
    return out;
}

OutStream& operator <<(OutStream& out, const void *ptr) {
    char buf[20];
    snprintf(buf, sizeof buf, "%p", ptr);
    return out << buf;
}


OutStream& operator <<(OutStream& out, const StreamEndl&) {
    out << "\n";
    out.flush();
    return out;
}

FileStream::FileStream(FileStream::FILE *file) :
    _file(file)
{}

FileStream::FileStream(const std::string& path, const std::string& mode) :
    _file(0)
{
    open(path, mode);
}

FileStream::~FileStream() {
    close();
}

bool FileStream::open(const std::string& path, const std::string& mode) {
    if (isOpen())
        return false;
    _file = castFILE(fopen(path.c_str(), mode.c_str()));
    return _file != 0;
}

StreamResult FileStream::basic_read(size& s, char *buf) {
    size_t n = UNSIZE(s);
    size_t k = fread(reinterpret_cast<void *>(buf), 1, n, castFILE(_file));
    s = SIZE(k);
    if (n == k)
        return StreamResult::OK;
    if (feof(castFILE(_file)))
        return StreamResult::EOF;
    if (ferror(castFILE(_file))) {
        return StreamResult::Error;
    }

    ASSERT_FAIL();
}

StreamResult FileStream::basic_write(size& s, const char *buf) {
    size_t n = UNSIZE(s);
    size_t k = fwrite(reinterpret_cast<const void *>(buf), 1, n, castFILE(_file));
    s = SIZE(k);
    if (n == k)
        return StreamResult::OK;
    if (feof(castFILE(_file)))
        return StreamResult::EOF;
    if (ferror(castFILE(_file)))
        return StreamResult::Error;

    ASSERT_FAIL();
}

StreamResult FileStream::basic_close() {
    StreamResult ret = basic_flush();
    fclose(castFILE(_file));
    return ret;
}

StreamResult FileStream::basic_flush() {
#ifdef SYSTEM_UNIX
    int ret;
    
    while ((ret = fflush(castFILE(_file))) != 0 && errno == EINTR)
        errno = 0;

    if (ret == 0)
        return StreamResult::OK;

    int err = errno;
    errno = 0;
    
    if (err == EAGAIN || err == EWOULDBLOCK)
        return StreamResult::Blocked;
    
    if (feof(castFILE(_file)))
        return StreamResult::EOF;
    return StreamResult::Error;

#else

    if (fflush(castFILE(_file)) == 0)
        return StreamResult::OK;
    if (feof(castFILE(_file)))
        return StreamResult::EOF;
    else
        return StreamResult::Error;
#endif
}

NullStream::NullStream() {
    close();
}

StreamResult NullStream::basic_flush() {
    return StreamResult::OK;
}

StreamResult NullStream::basic_read(size& s, char *) {
    s = 0;
    return StreamResult::EOF;
}

StreamResult NullStream::basic_write(size& s, const char *) {
    s = 0;
    return StreamResult::EOF;
}

StreamResult NullStream::basic_close() {
    return StreamResult::OK;
}

CooperativeInStream::CooperativeInStream(InStream *_in, Fiber *ioh, Fiber *su) :
    in(_in), io_handler(ioh), stream_user(su)
{}

StreamResult CooperativeInStream::basic_close_in(bool) {
    in->close();
    // FIXME: return real value
    return StreamResult::OK;
}

StreamResult CooperativeInStream::basic_read(size& s, char *buf) {
    size n = s;
    for (;;) {
        s = n;
        StreamResult res = in->read(s, buf);
        if (res == StreamResult::OK)
            return res;
        else if (res == StreamResult::Blocked) 
            fiber_switch(stream_user, io_handler);
        else
            return res; // error
    }
}

ByteStream::ByteStream(defs::size bufsize) :
    _buffer(),
    _read_cursor(0)
{
    _buffer.reserve(bufsize);
}

ByteStream::ByteStream(const char *buf, defs::size sz) :
    ByteStream(sz)
{
    write(sz, buf);
}

ByteStream::ByteStream(const std::string& str) :
    ByteStream(str.data(), str.size())
{}

ByteStream::~ByteStream() {}

void ByteStream::truncate(defs::size sz) {
    _buffer.resize(UNSIZE(sz), 0);
    if (_read_cursor > sz)
        _read_cursor = sz;
}

StreamResult ByteStream::basic_flush() {
    return StreamResult::OK;
}

StreamResult ByteStream::basic_close() {
    return StreamResult::OK;
}

StreamResult ByteStream::basic_read(defs::size& s, char *buf) {
    index lim = _read_cursor + s;
    if (lim > size())
        lim = size();
    if (_read_cursor >= lim) {
        s = 0;
        return StreamResult::EOF;
    }
    s = lim - _read_cursor;
    memcpy(buf, data() + _read_cursor, s);
    _read_cursor += s;
    return StreamResult::OK;
}

StreamResult ByteStream::basic_write(defs::size& s, const char *buf) {
    index end = size();
    _buffer.resize(end + s, 0);
    memcpy(data() + end, buf, s);
    return StreamResult::OK;
}

} // namespace io

} // namespace sys
