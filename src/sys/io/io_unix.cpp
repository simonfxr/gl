#include "sys/io.hpp"

#include "err/err.hpp"
#include "sys/fs.hpp"
#include "util/bit_cast.hpp"

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

namespace sys::io {

namespace {

#define OPTION(a, o) (((a) & (o)) == (o))

int
convertMode(HandleMode m)
{
    int flags = 0;
    bool w = OPTION(m, HM_WRITE);
    bool r = OPTION(m, HM_READ);
    bool append = OPTION(m, HM_APPEND);

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
    HandleMode mode = 0;
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
    auto errid = std::exchange(errno, 0);

    if (errid == EAGAIN || errid == EWOULDBLOCK)
        return HandleError::BLOCKED;

    switch (errid) {
    case ECONNRESET:
        return HandleError::EOF;
    case ENOENT:
        return HandleError::UNKNOWN;
    case EBADF:
        DEBUG_ERR(strerror(errid));
        return HandleError::BAD_HANDLE;
    default:
        DEBUG_ERR(strerror(errid));
        return HandleError::UNKNOWN;
    }
}

SocketError
convertErrnoSock()
{
    int errid = errno;
    errno = 0;
    switch (errid) {
    case EWOULDBLOCK:
        return SocketError::BLOCKED;
    case EBADF:
        DEBUG_ERR(strerror(errid));
        return SocketError::BAD_HANDLE;
    default:
        DEBUG_ERR(strerror(errid));
        return SocketError::UNKNOWN;
    }
}

HandleError
handleFromFD(int fd, Handle *h)
{
    int flags;
    h->_os.fd = fd;
    h->_mode = 0;
    RETRY_INTR(flags = fcntl(fd, F_GETFL));
    if (flags == -1)
        return convertErrno();
    h->_mode = unconvertMode(flags);
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

std::optional<Handle>
open(std::string_view path, HandleMode mode, HandleError &err)
{
    int flags = convertMode(mode);
    if (flags == -1) {
        err = HandleError::INVALID_PARAM;
        return std::nullopt;
    }

    int fd;
    auto strpath = std::string(path);
    RETRY_INTR(fd = ::open(strpath.c_str(), flags, 0777));

    if (fd == -1) {
        err = convertErrno();
        return std::nullopt;
    }
    Handle h;
    h._mode = mode;
    h._os.fd = fd;
    err = HandleError::OK;
    return { std::move(h) };
}

HandleMode
mode(Handle &h)
{
    ASSERT(h);
    return h._mode;
}

HandleError
elevate(Handle &h, HandleMode mode)
{
    ASSERT(h);
    int flags = convertMode(mode);
    if (flags == -1)
        return HandleError::INVALID_PARAM;

    int ret;
    RETRY_INTR(ret = fcntl(h._os.fd, F_SETFL, flags));
    if (ret == -1)
        return convertErrno();
    h._mode = unconvertMode(flags);
    return HandleError::OK;
}

HandleError
read(Handle &h, size_t &s, char *buf)
{
    ASSERT(h);
    auto n = s;
    ssize_t k;
    RETRY_INTR(k = ::read(h._os.fd, static_cast<void *>(buf), n));
    if (k >= 0) {
        s = k;
        return k == 0 ? HandleError::EOF : HandleError::OK;
    }
    s = 0;
    return convertErrno();
}

HandleError
write(Handle &h, size_t &s, const char *buf)
{
    ASSERT(h);
    auto n = s;
    ssize_t k;
    RETRY_INTR(k = ::write(h._os.fd, buf, n));
    if (k >= 0) {
        s = k;
        return HandleError::OK;
    }
    s = 0;
    return convertErrno();
}

HandleError
close(Handle &h)
{
    ASSERT(h);
    int ret;
    RETRY_INTR(ret = ::close(h._os.fd));
    h._os = {};
    h._mode = {};
    if (ret == -1)
        return convertErrno();
    return HandleError::OK;
}

std::optional<Socket>
listen(SocketProto proto,
       const IPAddr4 &addr,
       uint16_t port,
       SocketMode mode,
       SocketError &err)
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
    if (sock == -1) {
        err = convertErrnoSock();
        return std::nullopt;
    }

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

        auto server_so = bit_cast<sockaddr>(server);

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

    {
        Socket s;
        s._os.fd = sock;
        err = SocketError::OK;
        return { std::move(s) };
    }
socket_err:
    ::close(sock);
    err = convertErrnoSock();
    return std::nullopt;
}

std::optional<Handle>
accept(Socket &s, SocketError &err)
{
    ASSERT(s);

    sockaddr client{};
    socklen_t clen = sizeof client;

    int c;

    // reason: #define SOCK_CLOEXEC SOCK_CLOEXEC
    BEGIN_NO_WARN_DISABLED_MACRO_EXPANSION
    RETRY_INTR(c = accept4(s._os.fd, &client, &clen, SOCK_CLOEXEC));
    END_NO_WARN_DISABLED_MACRO_EXPANSION

    if (c == -1) {
        err = convertErrnoSock();
        return std::nullopt;
    }

    int flags;
    RETRY_INTR(flags = fcntl(c, F_GETFL));
    if (flags == -1)
        goto client_err;

    {
        err = SocketError::OK;
        Handle h;
        h._os.fd = c;
        h._mode = unconvertMode(flags);
        return { std::move(h) };
    }

client_err:
    ::close(c);
    err = convertErrnoSock();
    return std::nullopt;
}

SocketError
close(Socket &s)
{
    ASSERT(s);
    int ret;
    RETRY_INTR(ret = ::close(s._os.fd));
    s._os = {};
    if (ret == -1)
        return convertErrnoSock();
    return SocketError::OK;
}

} // namespace sys::io
