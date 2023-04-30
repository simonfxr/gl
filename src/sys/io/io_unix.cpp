#include "sys/io.hpp"

#include "err/err.hpp"
#include "sys/fs.hpp"
#include "sys/strerror_unix.hpp"

#include <bl/bit.hpp>

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace bl::literals;

namespace sys::io {

namespace {

#define OPTION(a, o) (((a) & (o)) == (o))

int
convertMode(HandleMode m)
{
    auto flags = 0;
    auto w = OPTION(m, HM_WRITE);
    auto r = OPTION(m, HM_READ);
    auto append = OPTION(m, HM_APPEND);

    if (w && r)
        flags |= O_RDWR | O_CREAT;
    else if (w)
        flags |= O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC);
    else if (r)
        flags |= O_RDONLY;
    else
        return -1;

    if (OPTION(m, HM_NONBLOCKING))
        flags |= O_NONBLOCK;

    return flags;
}

HandleMode
unconvertMode(int flags)
{
    auto mode = HandleMode{ 0 };
    if (OPTION(flags, O_RDWR))
        mode |= HM_READ | HM_WRITE;
    if (OPTION(flags, O_RDONLY))
        mode |= HM_READ;
    if (OPTION(flags, O_WRONLY))
        mode |= HM_WRITE;
    if (OPTION(flags, O_APPEND))
        mode |= HM_APPEND;
    if (OPTION(flags, O_NONBLOCK))
        mode |= HM_NONBLOCKING;
    return mode;
}

#define RETRY_INTR(op)                                                         \
    do {                                                                       \
        while ((op) == -1 && errno == EINTR) {                                 \
            errno = 0;                                                         \
            INFO("retrying after interrupt" #op);                              \
        }                                                                      \
    } while (0)

HandleError
convertErrno()
{
    auto errid = bl::exchange(errno, 0);

    if (errid == EAGAIN || errid == EWOULDBLOCK)
        return HandleError::BLOCKED;

    switch (errid) {
    case ECONNRESET:
        return HandleError::EOF;
    case ENOENT:
        return HandleError::UNKNOWN;
    case EBADF:
        DEBUG_ERR(strerror_errno(errid));
        return HandleError::BAD_HANDLE;
    case EWOULDBLOCK:
        return HandleError::BLOCKED;
    default:
        DEBUG_ERR(strerror_errno(errid));
        return HandleError::UNKNOWN;
    }
}

HandleError
handleFromFD(int fd, Handle *h)
{
    int flags;
    RETRY_INTR(flags = fcntl(fd, F_GETFL));
    if (flags < 0) {
        *h = Handle::fromNative(fd);
        return convertErrno();
    }
    *h = Handle::fromNative(fd, unconvertMode(flags));
    return HandleError::OK;
}
} // namespace

SYS_API Handle
stdin_handle()
{
    Handle h;
    auto err = handleFromFD(0, &h);
    if (err != HandleError::OK)
        ERR("error in fcntl");
    return h;
}

SYS_API Handle
stdout_handle()
{
    Handle h;
    auto err = handleFromFD(1, &h);
    if (err != HandleError::OK)
        ERR("error in fcntl");
    return h;
}

SYS_API Handle
stderr_handle()
{
    Handle h;
    auto err = handleFromFD(2, &h);
    if (err != HandleError::OK)
        ERR("error in fcntl");
    return h;
}

HandleResult<Handle>
open(bl::string_view path, HandleMode mode)
{
    auto flags = convertMode(mode);
    if (flags == -1)
        return util::unexpected{ HandleError::INVALID_PARAM };

    int fd;
    auto strpath = bl::string(path);
    RETRY_INTR(fd = ::open(strpath.c_str(), flags, 0777));

    if (fd == -1)
        return util::unexpected<HandleError>{ convertErrno() };
    return { Handle::fromNative(fd, mode) };
}

HandleError
elevate(Handle &h, HandleMode mode)
{
    ASSERT(h);
    auto flags = convertMode(mode);
    if (flags == -1)
        return HandleError::INVALID_PARAM;

    int ret;
    RETRY_INTR(ret = fcntl(h.native(), F_SETFL, flags));
    if (ret == -1)
        return convertErrno();
    h = Handle::fromNative(bl::move(h).takeNative(), mode);
    return HandleError::OK;
}

bl::pair<bl::usize, HandleError>
tag_invoke(read_f, Handle &h, bl::span<bl::byte> buf)
{
    ASSERT(h);
    auto n = buf.size();
    auto k = 0_iz;
    RETRY_INTR(k = ::read(h.native(), buf.data(), n));
    if (k >= 0)
        return { static_cast<bl::usize>(k),
                 k == 0 ? HandleError::EOF : HandleError::OK };
    return { 0, convertErrno() };
}

bl::pair<bl::usize, HandleError>
tag_invoke(write_f, Handle &h, bl::span<const bl::byte> buf)
{
    ASSERT(h);
    auto k = 0_iz;
    RETRY_INTR(k = ::write(h.native(), buf.data(), buf.size()));
    if (k >= 0)
        return { k, HandleError::OK };
    return { 0, convertErrno() };
}

HandleError
tag_invoke(close_f, Handle &h)
{
    ASSERT(h);
    auto fd = bl::move(h).takeNative();
    int ret{};
    RETRY_INTR(ret = ::close(fd));
    if (ret == -1)
        return convertErrno();
    return HandleError::OK;
}

HandleResult<Handle>
listen(SocketProto proto, const IPAddr4 &addr, uint16_t port, SocketMode mode)
{
    const int BACKLOG = 16;

    int type;
    int ret;

    // NOLINTNEXTLINE(hicpp-multiway-paths-covered)
    switch (proto) {
    case SP_TCP:
        type = SOCK_STREAM;
        break;
    default:
        UNREACHABLE;
    }

    int sock;
    RETRY_INTR(sock = socket(PF_INET, type, 0));
    if (sock == -1)
        return util::unexpected{ convertErrno() };

    int enable = 1;
    RETRY_INTR(
      ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof enable));
    if (ret == -1)
        goto socket_err;

    {
        sockaddr_in server{};
        memset(&server, 0, sizeof server);
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = addr.addr4;
        server.sin_port = hton(port);

        auto server_so = bl::bit_cast<sockaddr>(server);

        RETRY_INTR(ret = bind(sock, &server_so, sizeof server));
        if (ret == -1)
            goto socket_err;
    }

    if (OPTION(mode, SM_NONBLOCKING)) {
        int flags;
        RETRY_INTR(flags = fcntl(sock, F_GETFL));
        if (flags == -1)
            goto socket_err;
        flags |= O_NONBLOCK;
        RETRY_INTR(ret = fcntl(sock, F_SETFL, flags));
        if (ret == -1)
            goto socket_err;
    }

    if (type == SOCK_STREAM) {
        RETRY_INTR(ret = ::listen(sock, BACKLOG));
        if (ret == -1)
            goto socket_err;
    }

    return { Handle::fromNative(sock) };
socket_err:
    ::close(sock);
    return util::unexpected{ convertErrno() };
}

HandleResult<Handle>
accept(Handle &s)
{
    ASSERT(s);

    sockaddr client{};
    socklen_t clen = sizeof client;

    int c;

    // reason: #define SOCK_CLOEXEC SOCK_CLOEXEC
    BEGIN_NO_WARN_DISABLED_MACRO_EXPANSION
    RETRY_INTR(c = accept4(s.native(), &client, &clen, SOCK_CLOEXEC));
    END_NO_WARN_DISABLED_MACRO_EXPANSION

    if (c == -1)
        return util::unexpected{ convertErrno() };

    int flags;
    RETRY_INTR(flags = fcntl(c, F_GETFL));
    if (flags == -1) {
        auto err = convertErrno();
        ::close(c);
        return util::unexpected{ err };
    }

    return { Handle::fromNative(c, unconvertMode(flags)) };
}

} // namespace sys::io
