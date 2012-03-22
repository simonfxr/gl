#include "sys/io/Stream.hpp"
#include "err/err.hpp"

#include <string.h>
#include <errno.h>
#include <iostream>

namespace sys {

namespace io {

namespace {

struct Streams {
    FileStream stdout;
    FileStream stderr;

    Streams() :
        stdout(::stdout),
        stderr(::stderr)
    {
        stdout.dontClose(true);
        stderr.dontClose(true);
    }
};

Streams& streams() {
    static Streams *strs;
    if (strs == 0)
        strs = new Streams;
    return *strs;
}

} // namespace anon

OutStream& stdout() {
    return streams().stdout;
}

OutStream& stderr() {
    return streams().stderr;
}

// OutStream& endl(OutStream& out) {
//     out << "\n";
//     return out;
// }

struct StreamEndl {
    StreamEndl() {}
};
const StreamEndl endl;

OutStream& operator <<(OutStream& out, const std::string& str) {
    if (!out.outEOF()) {
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
        basic_state() |= SS_OUT_EOF;
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

StdInStream::StdInStream(std::istream& i) :
    _in(&i)
{
    if (_in->eof())
        basic_state() |= SS_IN_EOF;
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

FileStream::FileStream(FILE *_file) :
    file(_file)
{}

FileStream::FileStream(const std::string& path, const std::string& mode) {
    open(path, mode);
}

bool FileStream::open(const std::string& path, const std::string& mode) {
    if (isOpen())
        return false;
    file = fopen(path.c_str(), mode.c_str());
    return file != 0;
}



StreamResult FileStream::basic_read(size& s, char *buf) {
    size_t n = UNSIZE(s);
    size_t k = fread(reinterpret_cast<void *>(buf), 1, n, file);
    s = SIZE(k);
    if (n == k)
        return StreamOK;
    if (feof(file))
        return StreamEOF;
    if (ferror(file)) {
        return StreamError;
    }

    ASSERT_FAIL();
}

StreamResult FileStream::basic_write(size& s, const char *buf) {
    size_t n = UNSIZE(s);
    size_t k = fwrite(reinterpret_cast<const void *>(buf), 1, n, file);
    s = SIZE(k);
    if (n == k)
        return StreamOK;
    if (feof(file))
        return StreamEOF;
    if (ferror(file))
        return StreamError;

    ASSERT_FAIL();
}

void FileStream::basic_close() {
    fclose(file);
}

StreamResult FileStream::basic_flush() {
    int ret;
    
    while ((ret = fflush(file)) != 0 && errno == EINTR)
        errno = 0;

    if (ret == 0)
        return StreamOK;

    int err = errno;
    errno = 0;
    
    if (err == EAGAIN || err == EWOULDBLOCK)
        return StreamBlocked;
    
    if (feof(file))
        return StreamEOF;
    return StreamError;
}

StreamResult NullStream::basic_read(size& s, char *) {
    s = 0;
    return StreamEOF;
}

StreamResult NullStream::basic_write(size& s, const char *) {
    s = 0;
    return StreamEOF;
}

RecordingStream::RecordingStream(InStream& _in) :
    in(&_in),
    state(Recording),
    cursor(0)
{}

void RecordingStream::replay() {
    state = Replaying;
}

void RecordingStream::record() {
    state = Recording;
}

void RecordingStream::reset() {
    cursor = 0;
}

void RecordingStream::clear() {
    recorded.clear();
    cursor = 0;
}

void RecordingStream::basic_close() {
    in->close();
}

StreamResult RecordingStream::basic_read(size& s, char *b) {
    StreamResult res;
    
    switch (state) {
        
    case Recording: {
        
        res = in->read(s, b);
        for (defs::index i = 0; i < s; ++i)
            recorded.push_back(b[i]);
        break;
    }
    
    case Replaying: {
        
        size rem = SIZE(recorded.size()) - cursor;
        if (s <= rem) {
            memcpy(b, &recorded[cursor], s);
            cursor += s;
            res = StreamOK;
        } else {

            if (rem > 0) {
                memcpy(b, &recorded[cursor], rem);
                b += rem;
                s -= rem;
            }

            res = in->read(s, b);
            for (defs::index i = 0; i < s; ++i)
                recorded.push_back(b[i]);
            
            s += rem;

            cursor = 0;
            state = Recording;
        }

        break;
        
    }   
    default:
        ASSERT_FAIL();
    }

    return res;
}

} // namespace io

} // namespace sys
