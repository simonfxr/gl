#include "sys/io.hpp"

#include "err/err.hpp"
#include "sys/module.hpp"

#include <array>
#include <cstring>
#include <optional>
#include <vector>

namespace sys::io {

PP_DEF_ENUM_IMPL(SYS_HANDLE_ERROR_ENUM_DEF)
PP_DEF_ENUM_IMPL(SYS_SOCKET_ERROR_ENUM_DEF)

IO::IO() : ipa_any(0), ipa_local(127, 0, 0, 1) {}

HandleStream::HandleStream(Handle h) : handle(std::move(h)) {}

HandleStream::~HandleStream()
{
    close();
}

namespace {
StreamResult
toStreamResult(HandleError e)
{
    switch (e.value) {
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
    UNREACHABLE;
}
} // namespace

StreamResult
HandleStream::basic_close()
{
    if (!handle)
        return StreamResult::OK;
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
        std::tie(k, err) = sys::io::read(handle, std::span{ buf, k });
        read_cursor = 0;
        n += k;
    } else {
        size_t k = HANDLE_READ_BUFFER_SIZE;
        std::tie(k, err) = sys::io::read(handle, std::span{ read_buffer, k });
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
    auto res = toStreamResult(err);
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
        std::tie(k, err) = sys::io::write(handle, std::span{ write_buffer, k });
    }

    if (write_cursor == 0 ||
        (k == HANDLE_WRITE_BUFFER_SIZE && err == HandleError::OK)) {

        n += rem;
        k = s - rem;
        std::tie(k, err) = sys::io::write(handle, std::span{ buf + rem, k });
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
    return toStreamResult(err);
}

StreamResult
HandleStream::flush_buffer()
{
    auto [k, err] =
      sys::io::write(handle, std::span{ write_buffer, write_cursor });

    if (k > 0) {
        memmove(write_buffer, write_buffer + k, write_cursor - k);
        write_cursor -= k;
    }

    return toStreamResult(err);
}

HandleResult<HandleStream>
HandleStream::open(std::string_view path, HandleMode mode)
{
    auto res = sys::io::open(path, mode);
    if (!res)
        return util::unexpected{ res.error() };
    return { HandleStream{ std::move(res).value() } };
}

HandleResult<Array<char>>
readFile(std::string_view path, sys::io::OutStream &errout) noexcept
{
    static constexpr auto BUF_SIZE = size_t{ 8192 };
    auto res = open(path, HM_READ);
    auto err = HandleError{};
    if (!res) {
        err = res.error();
        goto fail;
    }
    {
        auto h = std::move(res).value();
        std::string str;
        for (;;) {
            std::array<char, BUF_SIZE> buf{};
            auto size = sizeof buf;
            std::tie(size, err) = read(h, std::span{ buf.data(), size });
            if (size > 0)
                str.insert(str.end(), buf.data(), buf.data() + size);
            if (err == HandleError::EOF) {
                err = HandleError::OK;
                return { { str.data(), str.size() } };
            }
            if (err != HandleError::OK)
                goto fail;
        }
    }
fail:
    if (errout.writeable())
        errout << "unable to read file: " << path << "\n";

    return util::unexpected{ err };
}

} // namespace sys::io
