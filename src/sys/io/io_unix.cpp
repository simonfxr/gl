#include "sys/io.hpp"
#include "err/err.hpp"

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

namespace sys {

namespace io {

using namespace defs;

namespace {

#define OPTION(a, o) (((a) & (o)) == (o))

int convertMode(HandleMode m) {
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

HandleMode unconvertMode(int flags) {
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

#define RETRY_INTR(op) do {                                     \
        while ((op) == -1 && errno == EINTR) {                  \
            errno = 0;                                          \
            INFO("retrying after interrupt" #op);               \
        }                                                       \
    } while (0)

HandleError convertErrno() {
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

SocketError convertErrnoSock() {
    int errid = errno;
    errno = 0;

    if (errid == EAGAIN || errid == EWOULDBLOCK)
        return SE_BLOCKED;

    switch (errid) {
    case EBADF:
        DEBUG_ERR(strerror(errid));
        return SE_BAD_SOCKET;
    default:
        DEBUG_ERR(strerror(errid));
        return SE_UNKNOWN;
    }
}

} // namespace anon

HandleError open(const std::string& path, HandleMode mode, Handle *h) {
    
    if (h == 0) {
        DEBUG_ERR("handle == 0");
        return HE_INVALID_PARAM;
    }
    
    int flags = convertMode(mode);
    if (flags == -1)
        return HE_INVALID_PARAM;
    
    int fd;
    RETRY_INTR(fd = ::open(path.c_str(), flags));

    if (fd == -1) {
        return convertErrno();
    } else {
        h->mode = mode;
        h->fd = fd;
        return HE_OK;
    }
}

HandleMode mode(Handle& h) {
    return h.mode;
}

HandleError elevate(Handle& h, HandleMode mode) {
    int flags = convertMode(mode);
    if (flags == -1)
        return HE_INVALID_PARAM;

    int ret;
    RETRY_INTR(ret = fcntl(h.fd, F_SETFL, flags));
    if (ret == -1) {
        return convertErrno();
    } else {
        h.mode = unconvertMode(flags);
        return HE_OK;
    }
}

HandleError read(Handle& h, size& s, char *buf) {
    size_t n = UNSIZE(s);
    ssize_t k;
    RETRY_INTR(k = ::read(h.fd, static_cast<void *>(buf), n));
    
    if (k > 0) {
        s = SIZE(k);
        return HE_OK;
    } else if (k == 0) {
        s = 0;
        return HE_EOF;
    } else {
        s = 0;
        return convertErrno();
    }
}

HandleError write(Handle& h, size& s, const char *buf) {
    size_t n = UNSIZE(s);
    ssize_t k;
    
    RETRY_INTR(k = ::write(h.fd, buf, n));
    
    if (k >= 0) {
        s = SIZE(k);
        return HE_OK;
    } else {
        s = 0;
        return convertErrno();
    }
}

HandleError close(Handle& h) {
    int ret;
    RETRY_INTR(ret = ::close(h.fd));
    if (ret == -1)
        return convertErrno();
    else
        return HE_OK;
}

SocketError listen(SocketProto proto, const IPAddr4& addr, uint16 port, SocketMode mode, Socket *s) {

    const int BACKLOG = 16;

    if (s == 0) {
        DEBUG_ERR("socket == 0");
        return SE_INVALID_PARAM;
    }
    
    int type;
    int protocol;
    int ret;

    switch (proto) {
    case SP_TCP:
        type = SOCK_STREAM;
//        protocol = IPPROTO_TCP;
        protocol = 0;
        break;
    default:
        DEBUG_ERR("invalid protocol");
        return SE_INVALID_PARAM;
    }
    
    int sock;
    RETRY_INTR(sock = socket(PF_INET, type, protocol));
    if (sock == -1)
        return convertErrnoSock();

    int enable = 1;
    RETRY_INTR(ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof enable));
    if (ret == -1)
        goto socket_err;

    struct sockaddr_in server;
    
    memset(&server, 0, sizeof server);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = addr.addr4;
    server.sin_port = hton(port);

    RETRY_INTR(ret = bind(sock, (struct sockaddr *)&server, sizeof server));
    if (ret == -1)
        goto socket_err;

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

SocketError accept(Socket& s, Handle *h) {
    
    if (h == 0) {
        DEBUG_ERR("handle == 0");
        return SE_INVALID_PARAM;
    }

    struct sockaddr_in client;
    socklen_t clen = sizeof client;

    int c;
    RETRY_INTR(c = ::accept(s.socket, (struct sockaddr *) &client, &clen));
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

SocketError close(Socket& s) {
    int ret;
    RETRY_INTR(ret = ::close(s.socket));
    if (ret == -1)
        return convertErrnoSock();
    else
        return SE_OK;
}

} // namespace io

} // namespace sys
