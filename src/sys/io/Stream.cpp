#include "sys/io/Stream.hpp"

#include "err/err.hpp"
#include "sys/module.hpp"

#include <array>
#include <cerrno>
#include <cstring>

namespace sys::io {

PP_DEF_ENUM_IMPL(SYS_STREAM_RESULT_ENUM_DEF)

namespace {

struct StreamState
{
    static StreamResult track(StreamFlags eof,
                              StreamFlags & /*flags*/,
                              StreamResult r);
};

StreamResult
StreamState::track(StreamFlags eof, StreamFlags &flags, StreamResult res)
{
    switch (res.value) {
    case StreamResult::OK:
    case StreamResult::Blocked:
        return res;
    case StreamResult::EOF:
        flags |= eof;
        return res;
    case StreamResult::Closed:
    case StreamResult::Error:
        return res;
    }
    UNREACHABLE;
}
} // namespace

InStream::InStream() = default;

InStream::~InStream() = default;

const StreamFlags &
InStream::rflags() const
{
    return _rflags;
}

StreamFlags &
InStream::rflags()
{
    return _rflags;
}

void
InStream::close()
{
    if (closed())
        return;

    StreamResult ret;
    if (closable()) {
        rflags() |= SF_IN_CLOSED;
        ret = basic_close_in(false);
    } else {
        ret = basic_close_in(true);
    }
    StreamState::track(SF_IN_EOF | SF_OUT_EOF, rflags(), ret);
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
    return StreamState::track(SF_IN_EOF, rflags(), basic_read(s, buf));
}

OutStream::OutStream() = default;

OutStream::~OutStream() = default;

void
OutStream::close()
{
    if (closed())
        return;

    StreamResult ret;
    if (closable()) {
        wflags |= SF_OUT_CLOSED;
        ret = basic_close_out();
    } else {
        ret = basic_flush();
    }
    StreamState::track(SF_IN_EOF | SF_OUT_EOF, wflags, ret);
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
    auto insize = s;
    auto err = StreamState::track(SF_OUT_EOF, wflags, basic_write(s, buf));
    if (err != StreamResult::OK || !line_buffered() ||
        !memchr(buf, '\n', insize))
        return err;
    return flush();
}

StreamResult
OutStream::flush()
{
    if (closed())
        return StreamResult::Closed;
    return StreamState::track(SF_OUT_EOF, wflags, basic_flush());
}

BEGIN_NO_WARN_UNINITIALIZED
IOStream::IOStream()
{
    closable(true);
}
END_NO_WARN_UNINITIALIZED

IOStream::~IOStream() = default;

const StreamFlags &
IOStream::rflags() const
{
    return wflags;
}

StreamFlags &
IOStream::rflags()
{
    return wflags;
}

StreamResult
IOStream::basic_close_in(bool flush_only)
{
    if (flush_only)
        return basic_flush();
    wflags |= SF_OUT_CLOSED;
    return basic_close();
}

StreamResult
IOStream::basic_close_out()
{
    wflags |= SF_IN_CLOSED;
    return basic_close();
}

Streams::Streams()
  : stdin(stdin_handle()), stdout(stdout_handle()), stderr(stderr_handle())
{
    stdin.closable(false);
    stdout.closable(false).line_buffered(true);
    stderr.closable(false).line_buffered(true);
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

#define DEF_PRINTF_WRITER(T, fmt, bufsz)                                       \
    StreamResult write_repr(OutStream &out, T value)                           \
    {                                                                          \
        if (!out.writeable())                                                  \
            return StreamResult::OK;                                           \
        std::array<char, bufsz> buf{};                                         \
        auto len = snprintf(buf.data(), sizeof buf, "%" fmt, value);           \
        return write_repr(out, std::string_view{ buf.data(), size_t(len) });   \
    }

DEF_PRINTF_WRITER(const void *, "p", 32)
DEF_PRINTF_WRITER(short, "hd", 16)
DEF_PRINTF_WRITER(unsigned short, "hu", 16)
DEF_PRINTF_WRITER(int, "d", 16)
DEF_PRINTF_WRITER(unsigned, "u", 16)
DEF_PRINTF_WRITER(long, "ld", 32)
DEF_PRINTF_WRITER(unsigned long, "lu", 32)
DEF_PRINTF_WRITER(long long, "lld", 32)
DEF_PRINTF_WRITER(unsigned long long, "llu", 32)
DEF_PRINTF_WRITER(float, ".4g", 16)
DEF_PRINTF_WRITER(double, ".6lg", 32)
DEF_PRINTF_WRITER(long double, ".6Lg", 32)

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

ByteStream::ByteStream(std::string_view str) : ByteStream(str.size())
{
    auto sz = str.size();
    write(sz, str.data());
}

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
    buffer.append(buf, s);
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

} // namespace sys::io
