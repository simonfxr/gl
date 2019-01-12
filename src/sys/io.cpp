#include "sys/io.hpp"

#include "err/err.hpp"
#include "sys/module.hpp"

#include <cstring>
#include <optional>
#include <vector>

namespace sys {
namespace io {

DEF_ENUM_CLASS_OPS(HandleError)
DEF_ENUM_CLASS_OPS(SocketError)

namespace {
StreamResult
convertErr(HandleError err)
{
    switch (err) {
    case HandleError::OK:
        return StreamResult::OK;
    case HandleError::BLOCKED:
        return StreamResult::Blocked;
    case HandleError::EOF:
        return StreamResult::EOF;
    case HandleError::BAD_HANDLE:
    case HandleError::INVALID_PARAM:
    case HandleError::UNKNOWN:
        return StreamResult::Error;
    }

    ASSERT_FAIL();
}
} // namespace

IO::IO() : ipa_any(0), ipa_local(127, 0, 0, 1) {}

HandleStream::HandleStream(Handle h)
  : handle(std::move(h)), read_cursor(0), write_cursor(0)
{}

HandleStream::~HandleStream()
{
    close();
}

namespace {
StreamResult
toStreamResult(HandleError e)
{
    switch (e) {
    case HandleError::OK:
        return StreamResult::OK;
    case HandleError::EOF:
        return StreamResult::EOF;
    case HandleError::BLOCKED:
        return StreamResult::Blocked;
    case HandleError::BAD_HANDLE:
        return StreamResult::Closed;
    case HandleError::INVALID_PARAM:
        [[fallthrough]];
    case HandleError::UNKNOWN:
        return StreamResult::Error;
    }
    CASE_UNREACHABLE;
}
} // namespace

StreamResult
HandleStream::basic_close()
{
    StreamResult ret1 = basic_flush();
    auto ret2 = sys::io::close(handle);
    return ret1 == StreamResult::OK ? toStreamResult(ret2) : ret1;
}

StreamResult
HandleStream::basic_flush()
{
    if (write_cursor > 0)
        return flush_buffer();

    return StreamResult::OK;
}

StreamResult
HandleStream::basic_read(size_t &s, char *buf)
{
    if (s == 0)
        return StreamResult::OK;

    if (s <= read_cursor) {
        memcpy(buf, read_buffer, s);
        read_cursor -= s;
        if (read_cursor > 0)
            memmove(read_buffer, read_buffer + s, read_cursor);

        ASSERT(s > 0);
        return StreamResult::OK;
    }

    size_t n = read_cursor;
    memcpy(buf, read_buffer, read_cursor);
    s -= read_cursor;
    buf += read_cursor;
    HandleError err;

    if (s > HANDLE_READ_BUFFER_SIZE) {
        size_t k = s;
        err = sys::io::read(handle, k, buf);
        read_cursor = 0;
        n += k;
    } else {
        size_t k = HANDLE_READ_BUFFER_SIZE;
        err = sys::io::read(handle, k, read_buffer);
        if (k > s) {
            n += s;
            read_cursor = k - s;
            memcpy(buf, read_buffer, s);
            memmove(read_buffer, read_buffer + s, k - s);

            if (err == HandleError::BLOCKED || err == HandleError::EOF)
                err = HandleError::OK;
        } else {
            read_cursor = 0;
            n += k;
            memcpy(buf, read_buffer, k);
        }
    }

    s = n;
    StreamResult res = convertErr(err);
    ASSERT(res != StreamResult::OK || s > 0);
    return res;
}

StreamResult
HandleStream::basic_write(size_t &s, const char *buf)
{
    size_t rem = HANDLE_WRITE_BUFFER_SIZE - write_cursor;

    if (s <= rem) {
        memcpy(write_buffer + write_cursor, buf, s);
        write_cursor += s;
        return StreamResult::OK;
    }

    size_t n = 0;
    size_t k = 0;
    HandleError err = HandleError::OK;

    if (write_cursor > 0) {
        memcpy(write_buffer + write_cursor, buf, rem);
        k = HANDLE_WRITE_BUFFER_SIZE;
        err = sys::io::write(handle, k, write_buffer);
    }

    if (write_cursor == 0 ||
        (k == HANDLE_WRITE_BUFFER_SIZE && err == HandleError::OK)) {

        n += rem;
        k = s - rem;
        err = sys::io::write(handle, k, buf + rem);
        n += k;
        const char *unwritten = buf + n;
        size_t rest = s - n;
        if (rest > 0) {
            size_t into = rest <= HANDLE_WRITE_BUFFER_SIZE
                            ? rest
                            : HANDLE_WRITE_BUFFER_SIZE;
            memcpy(write_buffer, unwritten, into);
            n += into;
            write_cursor = into;
        } else {
            write_cursor = 0;
        }
    } else {
        if (k > 0) {
            size_t end = HANDLE_WRITE_BUFFER_SIZE - k;
            size_t rest = k <= write_cursor ? s : s - (k - write_cursor);
            size_t into = rest <= k ? rest : k;

            memmove(write_buffer, write_buffer + k, end);
            memcpy(write_buffer + end, buf + rem, into);
            n += rem + into;
            write_cursor = end + into;
        } else {
            write_cursor = HANDLE_WRITE_BUFFER_SIZE;
        }
    }

    if (n == s && (err == HandleError::BLOCKED || err == HandleError::EOF))
        err = HandleError::OK;

    s = n;
    return convertErr(err);
}

StreamResult
HandleStream::flush_buffer()
{
    size_t k = write_cursor;
    HandleError err = sys::io::write(handle, k, write_buffer);

    if (k > 0) {
        memmove(write_buffer, write_buffer + k, write_cursor - k);
        write_cursor -= k;
    }

    return convertErr(err);
}

std::optional<HandleStream>
HandleStream::open(std::string_view path, HandleMode mode, HandleError &err)
{
    auto opt_h = sys::io::open(path, mode, err);
    if (!opt_h)
        return std::nullopt;
    return { std::move(opt_h).value() };
}

std::pair<std::unique_ptr<char[]>, size_t>
readFile(sys::io::OutStream &errout,
         std::string_view path,
         HandleError &err) noexcept
{
    auto opt_h = open(path, HM_READ, err);
    if (!opt_h)
        goto fail;
    {
        auto h = std::move(opt_h).value();
        std::vector<char> vec;
        for (;;) {
            char buf[8192];
            auto size = sizeof buf;
            err = read(h, size, buf);
            if (size > 0)
                vec.insert(vec.end(), buf, buf + size);
            if (err == HandleError::EOF) {
                auto data = std::make_unique<char[]>(vec.size() + 1);
                memcpy(data.get(), vec.data(), vec.size());
                data[vec.size()] = '\0';
                return std::make_pair(std::move(data), vec.size());
            }
            if (err != HandleError::OK)
                goto fail;
        }
    }
fail:
    if (errout.writeable())
        errout << "unable to read file: " << path << sys::io::endl;

    return std::make_pair(nullptr, 0);
} // namespace io

} // namespace io
} // namespace sys
