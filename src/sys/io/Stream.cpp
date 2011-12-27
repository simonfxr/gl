#include "sys/io/Stream.hpp"
#include "err/err.hpp"

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

Streams streams;

} // namespace anon

OutStream& stdout() {
    return streams.stdout;
}

OutStream& stderr() {
    return streams.stderr;
}

OutStream& endl(OutStream& out) {
    out << "\n";
    return out;
}

StdOutStream::StdOutStream(std::ostream& _out)
    : out(_out)
{
    if (_out.eof())
        this->basic_state(this->basic_state() | SS_OUT_EOF);
}

StreamResult StdOutStream::basic_write(size& s, const char *b) {
    std::streamsize n = s;
    out.write(b, n);
    if (out.good())
        return StreamOK;
    s = 0; // FIXME: how many were written?
    if (out.eof())
        return StreamEOF;
    return StreamError;
}

void StdOutStream::basic_flush() {
    out.flush();
}

FileStream::FileStream(FILE *_file) :
    file(_file)
{}

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

void FileStream::basic_flush() {
    fflush(file);
}

} // namespace io

} // namespace sys
