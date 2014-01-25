#include "sys/io/Stream.hpp"
#include "err/err.hpp"
#include "sys/module.hpp"

#include <cstring>
#include <cstdio>

#ifdef SYSTEM_UNIX
#  include <errno.h>
#endif

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

} // namespace anon

Streams::Streams() :
    stdout(castFILE(STDOUT_FILE)),
    stderr(castFILE(STDERR_FILE))
{
    stdout.dontClose(true);
    stderr.dontClose(true);
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
    if (!out.write_state().outEOF()) {
        size s = SIZE(str.size());
        out.write(s, str.data());
    }
    return out;
}

OutStream& operator <<(OutStream& out, const char *str) {
    if (str == 0)
        return out;
    size s = SIZE(strlen(str));
    out.write(s, str);
    return out;
}

OutStream& operator <<(OutStream& out, const StreamEndl&) {
    out << "\n";
    out.flush();
    return out;
}

OutStream& operator <<(OutStream& out, std::ostringstream& s) {
    return out << s.str();
}

OutStream& operator <<(OutStream& out, std::stringstream& s) {
    return out << s.str();
}

// OutStream& operator <<(OutStream& out, std::stringbuf *buf) {
//     if (buf != 0)
//         out << *buf;
//     return out;
// }

// OutStream& operator <<(OutStream& out, std::stringbuf& b) {
//     size s = SIZE(b.epptr() - b.pbase());
//     out.write(s, b.pbase());
//     return out;
// }

FileStream::FileStream(FileStream::FILE *file) :
    _file(file)
{}

FileStream::FileStream(const std::string& path, const std::string& mode) :
    _file(0)
{
    open(path, mode);
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
        return StreamOK;
    if (feof(castFILE(_file)))
        return StreamEOF;
    if (ferror(castFILE(_file))) {
        return StreamError;
    }

    ASSERT_FAIL();
}

StreamResult FileStream::basic_write(size& s, const char *buf) {
    size_t n = UNSIZE(s);
    size_t k = fwrite(reinterpret_cast<const void *>(buf), 1, n, castFILE(_file));
    s = SIZE(k);
    if (n == k)
        return StreamOK;
    if (feof(castFILE(_file)))
        return StreamEOF;
    if (ferror(castFILE(_file)))
        return StreamError;

    ASSERT_FAIL();
}

void FileStream::basic_close() {
    fclose(castFILE(_file));
}

StreamResult FileStream::basic_flush() {
#ifdef SYSTEM_UNIX
    int ret;
    
    while ((ret = fflush(castFILE(_file))) != 0 && errno == EINTR)
        errno = 0;

    if (ret == 0)
        return StreamOK;

    int err = errno;
    errno = 0;
    
    if (err == EAGAIN || err == EWOULDBLOCK)
        return StreamBlocked;
    
    if (feof(castFILE(_file)))
        return StreamEOF;
    return StreamError;

#else

    if (fflush(castFILE(_file)) == 0)
        return StreamOK;
    if (feof(castFILE(_file)))
        return StreamEOF;
    else
        return StreamError;
#endif
}

StreamResult NullStream::basic_read(size& s, char *) {
    s = 0;
    return StreamEOF;
}

StreamResult NullStream::basic_write(size& s, const char *) {
    s = 0;
    return StreamEOF;
}

void NullStream::basic_close() {}

CooperativeInStream::CooperativeInStream(InStream *_in, Fiber *ioh, Fiber *su) :
    in(_in), io_handler(ioh), stream_user(su)
{}

void CooperativeInStream::basic_close() {
    in->read_state().close();
}

StreamResult CooperativeInStream::basic_read(size& s, char *buf) {
    size n = s;
    for (;;) {
        s = n;
        StreamResult res = in->read(s, buf);
        if (res == StreamOK)
            return res;
        else if (res == StreamBlocked) 
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

void ByteStream::basic_close() {}

StreamResult ByteStream::basic_read(defs::size& s, char *buf) {
    index lim = _read_cursor + s;
    if (lim > size())
        lim = size();
    if (_read_cursor >= lim) {
        s = 0;
        return StreamEOF;
    }
    s = lim - _read_cursor;
    memcpy(buf, data() + _read_cursor, s);
    _read_cursor += s;
    return StreamOK;
}

StreamResult ByteStream::basic_write(defs::size& s, const char *buf) {
    index end = size();
    _buffer.resize(end + s, 0);
    memcpy(data() + end, buf, s);
    return StreamOK;
}

} // namespace io

} // namespace sys
