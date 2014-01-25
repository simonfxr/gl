#include "sys/io/Stream.hpp"
#include "err/err.hpp"
#include "sys/module.hpp"

#include <cstring>
#include <cstdio>
#include <iostream>

#ifdef SYSTEM_UNIX
#  include <errno.h>
#endif

namespace sys {

namespace io {

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


StdOutStream::StdOutStream(std::ostream& o)
    : _out(&o)
{
    if (o.eof())
        this->_state |= SS_OUT_EOF;
}

StreamResult StdOutStream::basic_write(size& s, const char *b) {
    std::streamsize n = UNSIZE(s);
    _out->write(b, n);
    if (_out->good())
        return StreamOK;
    s = 0; // FIXME: how many were written?
    if (_out->eof())
        return StreamEOF;
    return StreamError;
}

StreamResult StdOutStream::basic_flush() {
    _out->flush();
    if (_out->good())
        return StreamOK;
    if (_out->eof())
        return StreamEOF;
    return StreamError;
}

void StdOutStream::basic_close() {}

StdInStream::StdInStream(std::istream& i) :
    _in(&i)
{
    if (_in->eof())
        this->_state |= SS_IN_EOF;
}

StreamResult StdInStream::basic_read(size& s, char *b) {
    std::streamsize n = UNSIZE(s);
    _in->read(b, n);
    if (_in->good())
        return StreamOK;
    s = 0;
    if (_in->eof())
        return StreamEOF;
    return StreamError;
}

void StdInStream::basic_close() {}

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

} // namespace io

} // namespace sys
