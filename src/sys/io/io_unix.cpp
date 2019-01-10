#include "err/err.hpp"
#include "sys/io.hpp"

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

namespace sys {
namespace io {

namespace {

#define OPTION(a, o) (((a) & (o)) == (o))

int
convertMode(HandleMode m)
{
    int flags = 0;
    bool w = OPTION(m, HM_WRITE);
    bool r = OPTION(m, HM_READ);

    if (w && r)
        flags |= O_RDWR | O_CREAT;
    else if (w)
        flags |= O_WRONLY | O_CREAT;
    else if (r)
        flags |= O_RDONLY;
    else
        return -1;

    if (OPTION(m, HM_APPEND))
        flags |= O_APPEND;

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
    int errid = errno;
    errno = 0;

    if (errid == EAGAIN || errid == EWOULDBLOCK)
        return HE_BLOCKED;

    switch (errid) {
    case ECONNRESET:
        return HE_EOF;
    case EBADF:
        DEBUG_ERR(strerror(errid));
        return HE_BAD_HANDLE;
    default:
        DEBUG_ERR(strerror(errid));
        return HE_UNKNOWN;
    }
}

SocketError
convertErrnoSock()
{
    int errid = errno;
    errno = 0;
    switch (errid) {
    case EAGAIN:
    case EWOULDBLOCK:
        return SE_BLOCKED;
    case EBADF:
        DEBUG_ERR(strerror(errid));
        return SE_BAD_SOCKET;
    default:
        DEBUG_ERR(strerror(errid));
        return SE_UNKNOWN;
    }
}

HandleError
handleFromFD(int fd, Handle *h)
{
    int flags;
    h->fd = fd;
    h->mode = 0;
    RETRY_INTR(flags = fcntl(fd, F_GETFL));
    if (flags == -1)
        return convertErrno();
    h->mode = unconvertMode(flags);
    return HE_OK;
}
} // namespace

SYS_API Handle
stdin_handle()
{
    Handle h;
    auto err = handleFromFD(0, &h);
    if (err != HE_OK)
        ERR("error in fcntl");
    return h;
}

SYS_API Handle
stdout_handle()
{
    Handle h;
    auto err = handleFromFD(1, &h);
    if (err != HE_OK)
        ERR("error in fcntl");
    return h;
}

SYS_API Handle
stderr_handle()
{
    Handle h;
    auto err = handleFromFD(2, &h);
    if (err != HE_OK)
        ERR("error in fcntl");
    return h;
}

HandleError
open(std::string_view path, HandleMode mode, Handle *h)
{
    ASSERT(h);

    int flags = convertMode(mode);
    if (flags == -1)
        return HE_INVALID_PARAM;

    int fd;
    RETRY_INTR(fd = ::open(path.c_str(), flags));

    if (fd == -1)
        return convertErrno();
    h->_mode = mode;
    h->_fd = fd;
    return HE_OK;
}

HandleMode
mode(Handle &h)
{
    ASSERT(h);
    return h.mode;
}

HandleError
elevate(Handle &h, HandleMode mode)
{
    ASSERT(h);
    int flags = convertMode(mode);
    if (flags == -1)
        return HE_INVALID_PARAM;

    int ret;
    RETRY_INTR(ret = fcntl(h.fd, F_SETFL, flags));
    if (ret == -1)
        return convertErrno();
    h.mode = unconvertMode(flags);
    return HE_OK;
}

HandleError
read(Handle &h, size_t &s, char *buf)
{
    ASSERT(h);
    auto n = s;
    ssize_t k;
    RETRY_INTR(k = ::read(h.fd, static_cast<void *>(buf), n));
    if (k >= 0) {
        s = k;
        return k == 0 ? HE_EOF : HE_OK;
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
    RETRY_INTR(k = ::write(h.fd, buf, n));
    if (k >= 0) {
        s = k;
        return HE_OK;
    }
    s = 0;
    return convertErrno();
}

HandleError
close(Handle &h)
{
    ASSERT(h);
    int ret;
    RETRY_INTR(ret = ::close(h.fd));
    if (ret == -1)
        return convertErrno();
    return HE_OK;
}

SocketError
listen(SocketProto proto,
       const IPAddr4 &addr,
       uint16_t port,
       SocketMode mode,
       Socket *s)
{
    const int BACKLOG = 16;
    ASSERT(s);

    int type;
    int ret;

    switch (proto) {
    case SP_TCP:
        type = SOCK_STREAM;
        break;
    default:
        ASSERT_FAIL();
    }

    int sock;
    RETRY_INTR(sock = socket(PF_INET, type, 0));
    if (sock == -1)
        return convertErrnoSock();

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

        RETRY_INTR(ret = bind(sock,
                              reinterpret_cast<sockaddr *>(&server),
                              sizeof server));
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

    s->socket = sock;
    return SE_OK;

socket_err:
    ::close(sock);
    return convertErrnoSock();
}

SocketError
accept(Socket &s, Handle *h)
{
    ASSERT(s);
    ASSERT(h);

    sockaddr_in client{};
    socklen_t clen = sizeof client;

    int c;
    // reason: #define SOCK_CLOEXEC SOCK_CLOEXEC
    BEGIN_NO_WARN_DISABLED_MACRO_EXPANSION
    RETRY_INTR(c = accept4(s.socket,
                           reinterpret_cast<sockaddr *>(&client),
                           &clen,
                           SOCK_CLOEXEC));
    END_NO_WARN_DISABLED_MACRO_EXPANSION
    if (c == -1)
        return convertErrnoSock();

    int flags;
    RETRY_INTR(flags = fcntl(c, F_GETFL));
    if (flags == -1)
        goto client_err;

    h->fd = c;
    h->mode = unconvertMode(flags);
    return SE_OK;

client_err:
    ::close(c);
    return convertErrnoSock();
}

SocketError
close(Socket &s)
{
    ASSERT(s);
    int ret;
    RETRY_INTR(ret = ::close(s.socket));
    if (ret == -1)
        return convertErrnoSock();
    return SE_OK;
}

} // namespace io

} // namespace sys
