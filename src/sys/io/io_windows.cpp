#include "sys/io.hpp"

#include "err/err.hpp"
#include "sys/module.hpp"
#include "sys/win_utf_conv.hpp"

#include <WS2tcpip.h>
#include <WinSock2.h>
#include <Windows.h>
#include <cassert>
#include <cstdio>
#include <exception>

namespace sys {
namespace io {

using sys::win::utf16To8;
using sys::win::utf8To16;

namespace {
DWORD
handleModeToDWORD(HandleMode mode)
{
    DWORD m = 0;
    if (mode & HM_APPEND) {
        m = FILE_APPEND_DATA;
    } else {
        if (mode & HM_READ)
            m |= GENERIC_READ;
        else if (mode & HM_WRITE)
            m |= GENERIC_WRITE;
    }

    if (mode & HM_NONBLOCKING)
        WARN("HM_NONBLOCKING is ignored on files");
    return m;
}

void *
castFromHandle(HANDLE h)
{
    return reinterpret_cast<void *>(h);
}

HANDLE
castToHandle(void *h)
{
    return reinterpret_cast<HANDLE>(h);
}

void *
castFromSocket(SOCKET s)
{
    return reinterpret_cast<void *>(s);
}

SOCKET
castToSocket(void *s)
{
    return reinterpret_cast<SOCKET>(s);
}

HandleError
getLastHandleError()
{
    auto err = GetLastError();
    switch (err) {
    case ERROR_HANDLE_EOF:
        return HE_EOF;
    case ERROR_INVALID_HANDLE:
    case ERROR_HANDLE_NO_LONGER_VALID:
        return HE_BAD_HANDLE;
    default:
        return HE_UNKNOWN;
    }
}

SocketError
convertSocketError(DWORD err)
{
    switch (err) {
    case WSAEWOULDBLOCK:
        return SE_BLOCKED;
    case WSAECONNRESET:
        return SE_EOF;
    case WSAENOTSOCK:
    case WSAEBADF:
        return SE_BAD_SOCKET;
    default:
        return SE_UNKNOWN;
    }
}

SocketError
getLastSocketError()
{
    return convertSocketError(WSAGetLastError());
}

} // namespace

WS32Init::WS32Init()
{
    WSADATA wsaData;
    auto ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
#ifndef NDEBUG
    assert(ret == 0 && "WSAStartup() failed");
#else
    if (ret != 0) {
        fprintf(stderr, "WSAStartup() failed, terminating");
        std::terminate();
    }
#endif
}

WS32Init::~WS32Init()
{
    WSACleanup();
}

SYS_API Handle
stdin_handle()
{
    Handle h;
    h._handle = castFromHandle(GetStdHandle(STD_INPUT_HANDLE));
    return h;
}

SYS_API Handle
stdout_handle()
{
    Handle h;
    h._handle = castFromHandle(GetStdHandle(STD_OUTPUT_HANDLE));
    return h;
}

SYS_API Handle
stderr_handle()
{
    Handle h;
    h._handle = castFromHandle(GetStdHandle(STD_ERROR_HANDLE));
    return h;
}

HandleError
open(std::string_view path, HandleMode mode, Handle *h)
{
    ASSERT(h);
    auto wpath = utf8To16(path);
    auto access = handleModeToDWORD(mode);
    auto share = FILE_SHARE_READ | FILE_SHARE_WRITE;
    auto create =
      (mode & HM_APPEND) || (mode & HM_WRITE) ? OPEN_ALWAYS : OPEN_EXISTING;
    auto hndl = CreateFileW(wpath.c_str(),
                            access,
                            share,
                            nullptr,
                            create,
                            FILE_ATTRIBUTE_NORMAL,
                            nullptr);
    if (hndl == INVALID_HANDLE_VALUE)
        return getLastHandleError();
    h->_handle = castFromHandle(hndl);
    h->_mode = mode;
    h->_is_socket = false;
    return HE_OK;
}

HandleMode
mode(Handle &h)
{
    ASSERT(h);
    return h._mode;
}

HandleError
elevate(Handle &h, HandleMode hm)
{
    ASSERT(h);
    if (!h._is_socket)
        return HE_UNKNOWN;
    auto do_shutdown = false;
    int sd_mode = 0;
    if ((h._mode & HM_READ) != (hm & HM_READ)) {
        if (hm & HM_READ)
            return HE_UNKNOWN;
        do_shutdown = true;
        sd_mode = SD_RECEIVE;
    }
    if ((h._mode & HM_WRITE) != (hm & HM_WRITE)) {
        if (hm & HM_WRITE)
            return HE_UNKNOWN;
        sd_mode = do_shutdown && sd_mode == SD_RECEIVE ? SD_BOTH : SD_SEND;
        do_shutdown = true;
    }
    if (do_shutdown) {
        if (shutdown(castToSocket(h._handle), sd_mode) == SOCKET_ERROR)
            return HandleError(getLastSocketError());
    }

    h._mode &= ~(HM_READ | HM_WRITE);
    h._mode |= (hm & (HM_READ | HM_WRITE));

    if ((h._mode & HM_NONBLOCKING) != (hm & HM_NONBLOCKING)) {
        unsigned long val = !!(hm & HM_NONBLOCKING);
        if (ioctlsocket(castToSocket(h._handle), FIONBIO, &val) == SOCKET_ERROR)
            return HandleError(getLastSocketError());
        h._mode &= ~HM_NONBLOCKING;
        h._mode |= hm & HM_NONBLOCKING;
    }
    return HE_OK;
}

HandleError
read(Handle &h, size_t &sz, char *data)
{
    ASSERT(h);
    if (sz == 0)
        return HE_OK;
    if (h._is_socket) {
        auto ret = recv(castToSocket(h._handle), data, int(sz), 0);
        sz = 0;
        if (ret == SOCKET_ERROR)
            return HandleError(getLastSocketError());
        if (ret == 0)
            return HE_EOF;
        sz = size_t(ret);
        return HE_OK;
    } else {
        DWORD nread;
        auto ret =
          ReadFile(castToHandle(h._handle), data, DWORD(sz), &nread, nullptr);
        sz = 0;
        if (ret == FALSE)
            return getLastHandleError();
        if (nread == 0)
            return HE_EOF;
        sz = size_t(nread);
        return HE_OK;
    }
}

HandleError
write(Handle &h, size_t &sz, const char *data)
{
    ASSERT(h);
    if (h._is_socket) {
        auto ret = send(castToSocket(h._handle), data, int(sz), 0);
        sz = 0;
        if (ret == SOCKET_ERROR)
            return HandleError(getLastSocketError());
        sz = size_t(ret);
        return HE_OK;
    } else {
        DWORD nwrit;
        auto ret =
          WriteFile(castToHandle(h._handle), data, DWORD(sz), &nwrit, nullptr);
        sz = 0;
        if (ret == FALSE)
            return getLastHandleError();
        sz = size_t(nwrit);
        return HE_OK;
    }
}

HandleError
close(Handle &h)
{
    ASSERT(h);
    auto hndl = castToHandle(h._handle);
    h._handle = nullptr;
    h._mode = {};
    h._is_socket = false;
    if (CloseHandle(hndl) != FALSE)
        return HE_OK;
    else
        return HE_UNKNOWN;
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
    switch (proto) {
    case SP_TCP:
        type = SOCK_STREAM;
        break;
    default:
        ASSERT_FAIL();
    }

    SOCKET sock;
    sock = socket(PF_INET, type, 0);
    if (sock == INVALID_SOCKET)
        return getLastSocketError();

    int enable = 1;
    auto ret = setsockopt(sock,
                          SOL_SOCKET,
                          SO_REUSEADDR,
                          reinterpret_cast<char *>(&enable),
                          sizeof enable);
    if (ret != 0)
        goto socket_err;

    {
        struct sockaddr_in server
        {};
        memset(&server, 0, sizeof server);
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = addr.addr4;
        server.sin_port = hton(port);

        ret = bind(
          sock, reinterpret_cast<struct sockaddr *>(&server), sizeof server);
        if (ret != 0)
            goto socket_err;
    }

    if (mode & SM_NONBLOCKING) {
        unsigned long opt = 1;
        if (ioctlsocket(sock, FIONBIO, &opt) != 0)
            goto socket_err;
    }

    if (type == SOCK_STREAM) {
        if (::listen(sock, BACKLOG) != 0)
            goto socket_err;
    }

    s->_socket = castFromSocket(sock);
    return SE_OK;

socket_err:
    auto err = GetLastError();
    closesocket(sock);
    return convertSocketError(err);
}

SocketError
accept(Socket &s, Handle *h)
{
    ASSERT(s);
    ASSERT(h);

    sockaddr_in client{};
    socklen_t clen = sizeof client;

    auto c = ::accept(
      castToSocket(s._socket), reinterpret_cast<sockaddr *>(&client), &clen);

    if (c == INVALID_SOCKET)
        return getLastSocketError();

    h->_handle = castFromSocket(c);
    h->_is_socket = true;
    h->_mode = HM_READ | HM_WRITE;
    return SE_OK;
}

SocketError
close(Socket &sock)
{
    ASSERT(sock);
    auto hndl = castToHandle(sock._socket);
    sock._socket = nullptr;
    if (CloseHandle(hndl) != FALSE)
        return SE_OK;
    else
        return SE_UNKNOWN;
}

} // namespace io
} // namespace sys
