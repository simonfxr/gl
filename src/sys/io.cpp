#include "sys/io.hpp"

#include "err/err.hpp"
#include "sys/module.hpp"

#include "bl/optional.hpp"
#include "bl/vector.hpp"
#include <cstring>

namespace sys {
namespace io {

PP_DEF_ENUM_IMPL(SYS_HANDLE_ERROR_ENUM_DEF)
PP_DEF_ENUM_IMPL(SYS_SOCKET_ERROR_ENUM_DEF)

IO::IO() : ipa_any(0), ipa_local(127, 0, 0, 1) {}

HandleStream::HandleStream(Handle h)
  : handle(bl::move(h)), read_cursor(0), write_cursor(0)
{}

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
    return toStreamResult(err);
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

    return toStreamResult(err);
}

bl::optional<HandleStream>
HandleStream::open(bl::string_view path, HandleMode mode, HandleError &err)
{
    auto opt_h = sys::io::open(path, mode, err);
    if (!opt_h)
        return bl::nullopt;
    return { bl::move(opt_h).value() };
}

bl::dyn_array<char>
readFile(sys::io::OutStream &errout,
         bl::string_view path,
         HandleError &err) noexcept
{
    auto opt_h = open(path, HM_READ, err);
    if (!opt_h)
        goto fail;
    {
        auto h = bl::move(opt_h).value();
        bl::string str;
        for (;;) {
            char buf[8192];
            auto size = sizeof buf;
            err = read(h, size, buf);
            if (size > 0)
                str += bl::string_view(buf, size);
            if (err == HandleError::EOF) {
                err = HandleError::OK;
                return bl::dyn_array<char>(str.data(), str.size());
            }
            if (err != HandleError::OK)
                goto fail;
        }
    }
fail:
    if (errout.writeable())
        errout << "unable to read file: " << path << sys::io::endl;

    return {};
} // namespace io

} // namespace io
} // namespace sys
