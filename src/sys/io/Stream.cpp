#include "sys/io/Stream.hpp"

#include "err/err.hpp"
#include "sys/module.hpp"

#include <bl/array.hpp>
#include <bl/tuple.hpp>

#include <cerrno>
#include <cstring>

using namespace bl::literals;

namespace sys::io {

// bl::pair<bl::usize, IOError>
// tag_invoke(write_f, OutStream &o, bl::byte_span buf)
// {
//     if (buf.empty())
//         return { 0, IOError::OK };
//     auto insize = buf.size();
//     auto s = buf.size();
//     auto err = o.basic_write(s, buf.data());
//     if (err != IOError::OK || !o.line_buffered() ||
//         !memchr(buf.data(), '\n', insize))
//         return { s, err };
//     return { s, flush(o) };
// }

Streams::Streams()
  : stdin(stdin_handle())
  , stdout(stdout_handle(), true /* line_buffered */)
  , stderr(stderr_handle(), true /* line_buffered */)
{}

SYS_API InStream
stdin()
{
    return mod->io_streams.stdin;
}

OutStream
stdout()
{
    return mod->io_streams.stdout;
}

OutStream
stderr()
{
    return mod->io_streams.stderr;
}

#define DEF_PRINTF_WRITER(T, fmt, bufsz)                                       \
    IOError write_repr(OutStream out, T value)                                 \
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

bl::pair<bl::usize, IOError>
tag_invoke(read_f, NullStream &, bl::mut_byte_span) noexcept
{
    return { 0, IOError::OK };
}

bl::pair<bl::usize, IOError>
tag_invoke(write_f, NullStream &, bl::byte_span s) noexcept
{
    return { s.size(), IOError::OK };
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

bl::pair<bl::usize, IOError>
tag_invoke(read_f, ByteStream &s, bl::mut_byte_span buf) noexcept
{
    auto lim = s.read_cursor + buf.size();
    if (lim > s.size())
        lim = bl::usize();
    if (s.read_cursor >= lim) {
        return { 0, IOError::EOF };
    }
    auto n = lim - s.read_cursor;
    memcpy(buf.data(), s.data() + s.read_cursor, n);
    s.read_cursor += n;
    return { n, IOError::OK };
}

bl::pair<bl::usize, IOError>
tag_invoke(write_f, ByteStream &s, bl::byte_span buf) noexcept
{
    s.buffer.append(buf.begin(), buf.end());
    return { buf.size(), IOError::OK };
}

CooperativeInStream::CooperativeInStream(ClosableInStream in,
                                         Fiber *ioh,
                                         Fiber *su)
  : in(bl::move(in)), io_handler(ioh), stream_user(su)
{}

IOError
tag_invoke(close_f, CooperativeInStream &s) noexcept
{
    return io::close(s.in);
}

bl::pair<bl::usize, IOError>
tag_invoke(read_f, CooperativeInStream &cs, bl::mut_byte_span buf) noexcept
{
    auto s = buf.size();
    auto n = s;
    for (;;) {
        s = n;
        auto res = IOError{};
        bl::tie(s, res) = io::read(cs.in, bl::span{ buf.data(), s });
        if (res == IOError::OK)
            return { s, res };
        if (res == IOError::BLOCKED)
            fiber_switch(cs.stream_user, cs.io_handler);
        else
            return { s, res };
    }
}

bl::pair<bl::usize, IOError>
BufferedStreamBase::read_from(bl::mut_byte_span buf,
                              bl::usize &pos,
                              InStream in,
                              bl::mut_byte_span data) noexcept
{
    if (data.empty())
        return { 0, IOError::OK };

    if (data.size() <= pos) {
        memcpy(data.data(), buf.data(), data.size());
        pos -= data.size();
        if (pos > 0)
            memmove(buf.data(), buf.data() + data.size(), pos);

        return { data.size(), IOError::OK };
    }

    auto n = pos;
    memcpy(data.data(), buf.data(), pos);
    pos = 0;
    data = data.subspan(pos);
    IOError err;

    if (data.size() > buf.size()) {
        auto k = 0_z;
        bl::tie(k, err) = sys::io::read(in, data);
        n += k;
    } else {
        auto k = 0_z;
        bl::tie(k, err) = sys::io::read(in, buf);
        if (k > data.size()) {
            n += data.size();
            pos = k - data.size();
            memcpy(data.data(), buf.data(), data.size());
            memmove(buf.data(), buf.data() + data.size(), k - data.size());

            if (err == IOError::BLOCKED || err == IOError::EOF)
                err = IOError::OK;
        } else {
            n += k;
            memcpy(data.data(), buf.data(), k);
        }
    }

    ASSERT(err != IOError::OK || n > 0);
    return { n, err };
}

bl::pair<bl::usize, IOError>
BufferedStreamBase::write_to(bl::mut_byte_span buf,
                             bl::usize &pos,
                             bool line_buffered,
                             OutStream os,
                             bl::byte_span data) noexcept
{

    auto rem = buf.size() - pos;

    if (data.size() <= rem) {
        if (data.size())
            memcpy(buf.data() + pos, data.data(), data.size());
        pos += data.size();
        if (line_buffered && data.size() &&
            memchr(data.data(), '\n', data.size()))
            flush_to(buf, pos, os);
        return { data.size(), IOError::OK };
    }

    auto n = 0_z;
    auto k = 0_z;
    IOError err = IOError::OK;

    if (pos > 0) {
        memcpy(buf.data() + pos, data.data(), rem);
        k = buf.size();
        bl::tie(k, err) = sys::io::write(os, buf);
    }

    if (pos == 0 || (k == buf.size() && err == IOError::OK)) {
        n += rem;
        k = data.size() - rem;
        bl::tie(k, err) = sys::io::write(os, bl::span{ data.data() + rem, k });
        n += k;
        auto unwritten = data.data() + n;
        auto rest = data.size() - n;
        if (rest > 0) {
            auto into = bl::min(rest, buf.size());
            memcpy(buf.data(), unwritten, into);
            n += into;
            pos = into;
        } else {
            pos = 0;
        }
    } else if (k > 0) {
        auto end = buf.size() - k;
        auto rest = k <= pos ? data.size() : data.size() - (k - pos);
        auto into = rest <= k ? rest : k;

        memmove(buf.data(), buf.data() + k, end);
        memcpy(buf.data() + end, data.data() + rem, into);
        n += rem + into;
        pos = end + into;
    } else {
        pos = buf.size();
    }

    if (n == data.size() && (err == IOError::BLOCKED || err == IOError::EOF))
        err = IOError::OK;

    if (err == IOError::OK && line_buffered &&
        memchr(data.data(), '\n', data.size()))
        flush_to(buf, pos, os);

    return { data.size(), err };
}

IOError
BufferedStreamBase::flush_to(bl::mut_byte_span buf,
                             bl::usize &cursor,
                             OutStream s) noexcept
{
    auto [k, err] = sys::io::write(s, bl::span{ buf.data(), cursor });

    if (k > 0) {
        memmove(buf.data(), buf.data() + k, cursor - k);
        cursor -= k;
    }

    return err;
}

// IOResult<HandleStream>
// HandleStream::open(bl::string_view path, HandleMode mode)
// {
//     auto res = sys::io::open(path, mode);
//     if (!res)
//         return util::unexpected{ res.error() };
//     return { HandleStream{ bl::move(res).value() } };
// }

} // namespace sys::io
