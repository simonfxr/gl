#include "sys/io/Stream.hpp"

#include "err/err.hpp"
#include "sys/module.hpp"

#include <bl/array.hpp>
#include <bl/tuple.hpp>

#include <cerrno>
#include <cstring>

namespace sys::io {

InStream::~InStream() = default;

OutStream::~OutStream() = default;

bl::pair<bl::usize, StreamResult>
tag_invoke(write_f, OutStream &o, bl::span<const bl::byte> buf)
{
    if (buf.empty())
        return { 0, StreamResult::OK };
    auto insize = buf.size();
    auto s = buf.size();
    auto err = o.basic_write(s, buf.data());
    if (err != StreamResult::OK || !o.line_buffered() ||
        !memchr(buf.data(), '\n', insize))
        return { s, err };
    return { s, flush(o) };
}

IOStream::~IOStream() = default;

StreamResult
IOStream::basic_close_in()
{
    return basic_close();
}

StreamResult
IOStream::basic_close_out()
{
    return basic_close();
}

Streams::Streams()
  : stdin(stdin_handle()), stdout(stdout_handle()), stderr(stderr_handle())
{
    stdout.line_buffered(true);
    stderr.line_buffered(true);
}

SYS_API InStream &
stdin()
{
    return mod->io_streams.stdin;
}

OutStream &
stdout()
{
    return mod->io_streams.stdout;
}

OutStream &
stderr()
{
    return mod->io_streams.stderr;
}

#define DEF_PRINTF_WRITER(T, fmt, bufsz)                                       \
    StreamResult write_repr(OutStream &out, T value)                           \
    {                                                                          \
        bl::array<char, bufsz> buf{};                                          \
        auto len = snprintf(buf.data(), sizeof buf, "%" fmt, value);           \
        return write_repr(out, bl::string_view{ buf.data(), bl::usize(len) }); \
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
    close(*this);
}

StreamResult
NullStream::basic_flush()
{
    return StreamResult::OK;
}

StreamResult
NullStream::basic_read(bl::usize &s, bl::byte * /*buf*/)
{
    s = 0;
    return StreamResult::EOF;
}

StreamResult
NullStream::basic_write(bl::usize &s, const bl::byte * /*buf*/)
{
    s = 0;
    return StreamResult::EOF;
}

StreamResult
NullStream::basic_close()
{
    return StreamResult::OK;
}

ByteStream::ByteStream(bl::usize bufsize) : read_cursor(0)
{
    buffer.reserve(bufsize);
}

ByteStream::ByteStream(bl::string_view str) : ByteStream(str.size())
{
    write(*this, bl::as_bytes(str));
}

ByteStream::~ByteStream() = default;

void
ByteStream::truncate(bl::usize sz)
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
ByteStream::basic_read(bl::usize &s, bl::byte *buf)
{
    auto lim = read_cursor + s;
    if (lim > size())
        lim = bl::usize();
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
ByteStream::basic_write(bl::usize &s, const bl::byte *buf)
{
    auto rng = bl::span{ buf, s };
    buffer.append(rng.begin(), rng.end());
    return StreamResult::OK;
}

CooperativeInStream::CooperativeInStream(InStream *in_, Fiber *ioh, Fiber *su)
  : in(in_), io_handler(ioh), stream_user(su)
{}

StreamResult
CooperativeInStream::basic_close_in()
{
    close(*in);
    // FIXME: return real value
    return StreamResult::OK;
}

StreamResult
CooperativeInStream::basic_read(bl::usize &s, bl::byte *buf)
{
    auto n = s;
    for (;;) {
        s = n;
        auto res = StreamResult{};
        bl::tie(s, res) = read(*in, bl::span{ buf, s });
        if (res == StreamResult::OK)
            return res;
        if (res == StreamResult::Blocked)
            fiber_switch(stream_user, io_handler);
        else
            return res; // error
    }
}

} // namespace sys::io
