#include "sys/io.hpp"
#include "err/err.hpp"
#include "sys/module.hpp"

#include <cstring>
#include <optional>

namespace sys {
namespace io {

using defs::size_t;

namespace {

StreamResult
convertErr(HandleError err)
{
    switch (err) {
    case HE_OK:
        return StreamResult::OK;
    case HE_BLOCKED:
        return StreamResult::Blocked;
    case HE_EOF:
        return StreamResult::EOF;
    case HE_BAD_HANDLE:
    case HE_INVALID_PARAM:
    case HE_UNKNOWN:
        return StreamResult::Error;
    }

    ASSERT_FAIL();
}

} // namespace

IO::IO() : ipa_any(0), ipa_local(127, 0, 0, 1) {}

const IPAddr4
IPA_ANY()
{
    return module->io.ipa_any;
}

const IPAddr4
IPA_LOCAL()
{
    return module->io.ipa_local;
}

HandleStream::HandleStream(const Handle &h)
  : handle(h), read_cursor(0), write_cursor(0)
{}

HandleStream::~HandleStream()
{
    close();
}

StreamResult
HandleStream::basic_close()
{
    StreamResult ret = basic_flush();
    sys::io::close(handle);
    return ret;
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
        memcpy(buf, read_buffer, UNSIZE(s));
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

            if (err == HE_BLOCKED || err == HE_EOF)
                err = HE_OK;
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
    HandleError err = HE_OK;

    if (write_cursor > 0) {
        memcpy(write_buffer + write_cursor, buf, rem);
        k = HANDLE_WRITE_BUFFER_SIZE;
        err = sys::io::write(handle, k, write_buffer);
    }

    if (write_cursor == 0 || (k == HANDLE_WRITE_BUFFER_SIZE && err == HE_OK)) {

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

    if (n == s && (err == HE_BLOCKED || err == HE_EOF))
        err = HE_OK;

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

namespace {
template<typename F>
struct Finally
{
    std::optional<F> _cleanup;

    Finally(F f) : _cleanup(std::move(f)) {}

    Finally(const Finally &) = delete;
    Finally(Finally &&) = default;

    ~Finally() { fire(); }

    Finally &operator=(const Finally &) = delete;
    Finally &operator=(Finally &&) = default;

    void fire()
    {
        if (_cleanup) {
            _cleanup->operator()();
            _cleanup = std::nullopt;
        }
    }

    void disarm() { _cleanup = std::nullopt; }
};
} // namespace

namespace {
template<typename F>
auto
make_finally(F &&f)
{
    return Finally<std::remove_cv_t<F>>(std::forward<F>(f));
}
} // namespace

std::pair<std::unique_ptr<char[]>, size_t>
readFile(sys::io::OutStream &err, const std::string &path) noexcept
{
    FILE *in = fopen(path.c_str(), "re");
    auto close_in = make_finally([in]() {
        if (in)
            fclose(in);
    });

    if (in == nullptr)
        goto fail;

    if (fseek(in, 0, SEEK_END) == -1)
        goto fail;

    {
        auto ssize = ftell(in);
        if (ssize < 0)
            goto fail;
        auto size = size_t(ssize);
        if (fseek(in, 0, SEEK_SET) == -1)
            goto fail;

        {
            auto contents = std::make_unique<char[]>(size + 1);
            if (fread(contents.get(), size, 1, in) != 1)
                goto fail;

            contents[size] = '\0';
            return std::make_pair(std::move(contents), size_t(size));
        }
    }

fail:
    if (err.writeable())
        err << "unable to read file: " << path << sys::io::endl;

    return std::make_pair(nullptr, 0);
}

} // namespace io
} // namespace sys
