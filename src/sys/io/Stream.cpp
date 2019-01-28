#include "sys/io/Stream.hpp"

#include "err/err.hpp"
#include "sys/io.hpp"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#undef EOF
#undef stdin
#undef stdout
#undef stderr

namespace sys {
namespace io {

PP_DEF_ENUM_IMPL(SYS_STREAM_RESULT_ENUM_DEF)

namespace {

StreamResult
track_state(StreamFlags eof, StreamFlags &flags, StreamResult res)
{
    switch (res.value) {
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
    UNREACHABLE;
}
} // namespace

InStream::InStream()
{
    this->flags() |= SF_CLOSABLE | SF_OUT_CLOSED;
}

InStream::~InStream() = default;

void
InStream::close()
{
    if (closed())
        return;

    StreamResult ret;
    if (closable()) {
        flags() |= SF_IN_CLOSED;
        ret = basic_close_in(false);
    } else {
        ret = basic_close_in(true);
    }
    track_state(SF_IN_EOF | SF_OUT_EOF, flags(), ret);
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
    return track_state(SF_IN_EOF, flags(), basic_read(s, buf));
}

OutStream::OutStream()
{
    flags() |= SF_CLOSABLE | SF_IN_CLOSED;
}

OutStream::~OutStream() = default;

void
OutStream::close()
{
    if (closed())
        return;

    StreamResult ret;
    if (closable()) {
        flags() |= SF_OUT_CLOSED;
        ret = basic_close_out();
    } else {
        ret = basic_flush();
    }
    track_state(SF_IN_EOF | SF_OUT_EOF, flags(), ret);
}

StreamResult
OutStream::write(size_t &s, const char *buf)
{
    if (!writable()) {
        s = 0;
        return closed() ? StreamResult::Closed : StreamResult::EOF;
    }
    if (s <= 0) {
        s = 0;
        return StreamResult::OK;
    }
    return track_state(SF_OUT_EOF, flags(), basic_write(s, buf));
}

StreamResult
OutStream::flush()
{
    if (closed())
        return StreamResult::Closed;
    return track_state(SF_OUT_EOF, flags(), basic_flush());
}

IOStream::IOStream()
{
    OutStream::flags() = SF_CLOSABLE;
}

IOStream::~IOStream() = default;

StreamResult
IOStream::basic_close_in(bool flush_only)
{
    if (flush_only) {
        return basic_flush();
    }
    flags() |= SF_OUT_CLOSED;
    return basic_close();
}

StreamResult
IOStream::basic_close_out()
{
    flags() |= SF_IN_CLOSED;
    return basic_close();
}

SYS_API InStream &
stdin()
{
    BEGIN_NO_WARN_GLOBAL_DESTRUCTOR
    static HandleStream stream(stdin_handle());
    END_NO_WARN_GLOBAL_DESTRUCTOR
    return stream;
}

OutStream &
stdout()
{
    BEGIN_NO_WARN_GLOBAL_DESTRUCTOR
    static HandleStream stream(stdout_handle());
    END_NO_WARN_GLOBAL_DESTRUCTOR
    return stream;
}

OutStream &
stderr()
{
    BEGIN_NO_WARN_GLOBAL_DESTRUCTOR
    static HandleStream stream(stderr_handle());
    END_NO_WARN_GLOBAL_DESTRUCTOR
    return stream;
}

struct StreamEndl
{};

const StreamEndl endl = {};

#define DEF_PRINTF_WRITER(T, fmt, bufsz)                                       \
    StreamResult write_repr(OutStream &out, T value)                           \
    {                                                                          \
        if (!out.writable())                                                   \
            return StreamResult::OK;                                           \
        char buf[bufsz];                                                       \
        auto len = snprintf(buf, sizeof buf, "%" fmt, value);                  \
        return write_repr(out, bl::string_view{ buf, size_t(len) });           \
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

ByteStream::ByteStream(bl::string_view str) : ByteStream(str.size())
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
    buffer += bl::string_view(buf, s);
    return StreamResult::OK;
}

CooperativeInStream::CooperativeInStream(InStream *in_, Fiber &ioh, Fiber &su)
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
            Fiber::switch_to(io_handler);
        else
            return res; // error
    }
}

} // namespace io
} // namespace sys
