#include "sys/io/Stream.hpp"

#include "err/err.hpp"
#include "sys/module.hpp"

#include <cerrno>
#include <cstring>

namespace sys {
namespace io {

namespace {
#if 0
FILE *
castFILE(FileStream::FILE *p)
{
    return reinterpret_cast<FILE *>(p);
}

FileStream::FILE *
castFILE(FILE *p)
{
    return reinterpret_cast<FileStream::FILE *>(p);
}
#endif
struct StreamState
{
    static StreamResult track(StreamFlags eof,
                              StreamFlags & /*flags*/,
                              StreamResult r);
};

StreamResult
StreamState::track(StreamFlags eof, StreamFlags &flags, StreamResult res)
{
    switch (res) {
    case StreamResult::OK:
        return res;
    case StreamResult::Blocked:
        return res;
    case StreamResult::EOF:
        flags |= eof;
        return res;
    case StreamResult::Closed:
        return res;
    case StreamResult::Error:
        return res;
    }
    CASE_UNREACHABLE;
}
} // namespace

InStream::InStream() : flags(_flags_store), _flags_store(SF_CLOSABLE) {}

InStream::InStream(StreamFlags &flags_) : flags(flags_), _flags_store(0) {}

InStream::~InStream() = default;

void
InStream::close()
{
    if (closed())
        return;

    StreamResult ret;
    if (closable()) {
        flags |= SF_IN_CLOSED;
        ret = basic_close_in(false);
    } else {
        ret = basic_close_in(true);
    }
    StreamState::track(SF_IN_EOF | SF_OUT_EOF, flags, ret);
}

StreamResult
InStream::read(size_t &s, char *buf)
{
    if (!readable()) {
        s = 0;
        return closed() ? StreamResult::Closed : StreamResult::EOF;
    }
    if (s <= 0) {
        s = 0;
        return StreamResult::OK;
    }
    return StreamState::track(SF_IN_EOF, flags, basic_read(s, buf));
}

OutStream::OutStream() : flags(SF_CLOSABLE) {}

OutStream::~OutStream() = default;

void
OutStream::close()
{
    if (closed())
        return;

    StreamResult ret;
    if (closable()) {
        flags |= SF_OUT_CLOSED;
        ret = basic_close_out();
    } else {
        ret = basic_flush();
    }
    StreamState::track(SF_IN_EOF | SF_OUT_EOF, flags, ret);
}

StreamResult
OutStream::write(size_t &s, const char *buf)
{
    if (!writeable()) {
        s = 0;
        return closed() ? StreamResult::Closed : StreamResult::EOF;
    }
    if (s <= 0) {
        s = 0;
        return StreamResult::OK;
    }
    return StreamState::track(SF_OUT_EOF, flags, basic_write(s, buf));
}

StreamResult
OutStream::flush()
{
    if (closed())
        return StreamResult::Closed;
    return StreamState::track(SF_OUT_EOF, flags, basic_flush());
}

BEGIN_NO_WARN_UNINITIALIZED
IOStream::IOStream() : InStream(flags) {}
END_NO_WARN_UNINITIALIZED

IOStream::~IOStream() = default;

StreamResult
IOStream::basic_close_in(bool flush_only)
{
    if (flush_only) {
        return basic_flush();
    }
    flags |= SF_OUT_CLOSED;
    return basic_close();
}

StreamResult
IOStream::basic_close_out()
{
    flags |= SF_IN_CLOSED;
    return basic_close();
}

Streams::Streams()
  : stdin(stdin_handle()), 
	stdout(stdout_handle()), 
	stderr(stderr_handle())
{
    stdin.closable(false);
    stdout.closable(false);
    stderr.closable(false);
}

SYS_API InStream &
stdin()
{
    return module->io_streams.stdin;
}

OutStream &
stdout()
{
    return module->io_streams.stdout;
}

OutStream &
stderr()
{
    return module->io_streams.stderr;
}

struct StreamEndl
{};
const StreamEndl endl = {};

OutStream &
operator<<(OutStream &out, const std::string &str)
{
    return out << std::string_view{ str };
}

OutStream &
operator<<(OutStream &out, std::string_view str)
{
    if (out.writeable()) {
        size_t s = str.size();
        out.write(s, str.data());
    }
    return out;
}

OutStream &
operator<<(OutStream &out, const char *str)
{
    if (!out.writeable())
        return out;
    if (str == nullptr)
        return out;
    size_t s = strlen(str);
    out.write(s, str);
    return out;
}

OutStream &
operator<<(OutStream &out, char c)
{
    if (!out.writeable())
        return out;
    size_t s = 1;
    out.write(s, &c);
    return out;
}

OutStream &
operator<<(OutStream &out, const void *ptr)
{
    char buf[20];
    snprintf(buf, sizeof buf, "%p", ptr);
    return out << buf;
}

OutStream &
operator<<(OutStream &out, const StreamEndl & /*unused*/)
{
    out << "\n";
    out.flush();
    return out;
}
#if 0
FileStream::FileStream(FileStream::FILE *file) : _file(file) {}

FileStream::FileStream(const std::string &path, const std::string &mode)
  : _file(nullptr)
{
    open(path, mode);
}

FileStream::~FileStream()
{
    close();
}

bool
FileStream::open(const std::string &path, const std::string &mode)
{
    if (isOpen())
        return false;
    _file = castFILE(fopen(path.c_str(), mode.c_str()));
    return _file != nullptr;
}

StreamResult
FileStream::basic_read(size_t &s, char *buf)
{
    size_t n = s;
    size_t k = fread(reinterpret_cast<void *>(buf), 1, n, castFILE(_file));
    s = k;
    if (n == k)
        return StreamResult::OK;
    if (feof(castFILE(_file)) != 0)
        return StreamResult::EOF;
    if (ferror(castFILE(_file)) != 0) {
        return StreamResult::Error;
    }

    ASSERT_FAIL();
}

StreamResult
FileStream::basic_write(size_t &s, const char *buf)
{
    size_t n = s;
    size_t k =
      fwrite(reinterpret_cast<const void *>(buf), 1, n, castFILE(_file));
    s = k;
    if (n == k)
        return StreamResult::OK;
    if (feof(castFILE(_file)) != 0)
        return StreamResult::EOF;
    if (ferror(castFILE(_file)) != 0)
        return StreamResult::Error;

    ASSERT_FAIL();
}

StreamResult
FileStream::basic_close()
{
    StreamResult ret = basic_flush();
    if (_file != nullptr)
        fclose(castFILE(_file));
    return ret;
}

StreamResult
FileStream::basic_flush()
{

    if (_file == nullptr)
        return StreamResult::Error;

#ifdef HU_OS_POSIX
    int ret;

    while ((ret = fflush(castFILE(_file))) != 0 && errno == EINTR)
        errno = 0;

    if (ret == 0)
        return StreamResult::OK;

    int err = errno;
    errno = 0;

    if (err == EAGAIN || err == EWOULDBLOCK)
        return StreamResult::Blocked;

    if (feof(castFILE(_file)) != 0)
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
#endif
NullStream::NullStream()
{
    close();
}

StreamResult
NullStream::basic_flush()
{
    return StreamResult::OK;
}

StreamResult
NullStream::basic_read(size_t &s, char * /*buf*/)
{
    s = 0;
    return StreamResult::EOF;
}

StreamResult
NullStream::basic_write(size_t &s, const char * /*buf*/)
{
    s = 0;
    return StreamResult::EOF;
}

StreamResult
NullStream::basic_close()
{
    return StreamResult::OK;
}

ByteStream::ByteStream(size_t bufsize) : read_cursor(0)
{
    buffer.reserve(bufsize);
}

ByteStream::ByteStream(const char *buf, size_t sz) : ByteStream(sz)
{
    write(sz, buf);
}

ByteStream::ByteStream(const std::string &str)
  : ByteStream(str.data(), str.size())
{}

ByteStream::~ByteStream() = default;

void
ByteStream::truncate(size_t sz)
{
    buffer.resize(sz, 0);
    if (read_cursor > sz)
        read_cursor = sz;
}

StreamResult
ByteStream::basic_flush()
{
    return StreamResult::OK;
}

StreamResult
ByteStream::basic_close()
{
    return StreamResult::OK;
}

StreamResult
ByteStream::basic_read(size_t &s, char *buf)
{
    size_t lim = read_cursor + s;
    if (lim > size())
        lim = size_t();
    if (read_cursor >= lim) {
        s = 0;
        return StreamResult::EOF;
    }
    s = lim - read_cursor;
    memcpy(buf, data() + read_cursor, s);
    read_cursor += s;
    return StreamResult::OK;
}

StreamResult
ByteStream::basic_write(size_t &s, const char *buf)
{
    size_t end = size();
    buffer.resize(end + s, 0);
    memcpy(data() + end, buf, s);
    return StreamResult::OK;
}

CooperativeInStream::CooperativeInStream(InStream *in_, Fiber *ioh, Fiber *su)
  : in(in_), io_handler(ioh), stream_user(su)
{}

StreamResult
CooperativeInStream::basic_close_in(bool /*flush_only*/)
{
    in->close();
    // FIXME: return real value
    return StreamResult::OK;
}

StreamResult
CooperativeInStream::basic_read(size_t &s, char *buf)
{
    size_t n = s;
    for (;;) {
        s = n;
        StreamResult res = in->read(s, buf);
        if (res == StreamResult::OK)
            return res;
        if (res == StreamResult::Blocked)
            fiber_switch(stream_user, io_handler);
        else
            return res; // error
    }
}

} // namespace io
} // namespace sys
