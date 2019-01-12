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
        return HandleError::EOF;
    case ERROR_INVALID_HANDLE:
        return HandleError::BAD_HANDLE;
    default:
        return HandleError::UNKNOWN;
    }
}

SocketError
convertSocketError(DWORD err)
{
    switch (err) {
    case WSAEWOULDBLOCK:
        return SocketError::BLOCKED;
    case WSAECONNRESET:
        return SocketError::EOF;
    case WSAENOTSOCK:
    case WSAEBADF:
        return SocketError::BAD_SOCKET;
    default:
        return SocketError::UNKNOWN;
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
    h._os.handle = castFromHandle(GetStdHandle(STD_INPUT_HANDLE));
    return h;
}

SYS_API Handle
stdout_handle()
{
    Handle h;
    h._os.handle = castFromHandle(GetStdHandle(STD_OUTPUT_HANDLE));
    return h;
}

SYS_API Handle
stderr_handle()
{
    Handle h;
    h._os.handle = castFromHandle(GetStdHandle(STD_ERROR_HANDLE));
    return h;
}

std::optional<Handle>
open(std::string_view path, HandleMode mode, HandleError &err)
{
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
    if (hndl == INVALID_HANDLE_VALUE) {
        err = getLastHandleError();
        return std::nullopt;
    }
    Handle h;
    h._os.handle = castFromHandle(hndl);
    h._mode = mode;
    h._os.is_socket = false;
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
elevate(Handle &h, HandleMode hm)
{
    ASSERT(h);
    if (!h._os.is_socket)
        return HandleError::UNKNOWN;
    auto do_shutdown = false;
    int sd_mode = 0;
    if ((h._mode & HM_READ) != (hm & HM_READ)) {
        if (hm & HM_READ)
            return HandleError::UNKNOWN;
        do_shutdown = true;
        sd_mode = SD_RECEIVE;
    }
    if ((h._mode & HM_WRITE) != (hm & HM_WRITE)) {
        if (hm & HM_WRITE)
            return HandleError::UNKNOWN;
        sd_mode = do_shutdown && sd_mode == SD_RECEIVE ? SD_BOTH : SD_SEND;
        do_shutdown = true;
    }
    if (do_shutdown) {
        if (shutdown(castToSocket(h._os.handle), sd_mode) == SOCKET_ERROR)
            return HandleError(getLastSocketError());
    }

    h._mode &= ~(HM_READ | HM_WRITE);
    h._mode |= (hm & (HM_READ | HM_WRITE));

    if ((h._mode & HM_NONBLOCKING) != (hm & HM_NONBLOCKING)) {
        unsigned long val = !!(hm & HM_NONBLOCKING);
        if (ioctlsocket(castToSocket(h._os.handle), FIONBIO, &val) ==
            SOCKET_ERROR)
            return HandleError(getLastSocketError());
        h._mode &= ~HM_NONBLOCKING;
        h._mode |= hm & HM_NONBLOCKING;
    }
    return HandleError::OK;
}

HandleError
read(Handle &h, size_t &sz, char *data)
{
    ASSERT(h);
    if (sz == 0)
        return HandleError::OK;
    if (h._os.is_socket) {
        auto ret = recv(castToSocket(h._os.handle), data, int(sz), 0);
        sz = 0;
        if (ret == SOCKET_ERROR)
            return HandleError(getLastSocketError());
        if (ret == 0)
            return HandleError::EOF;
        sz = size_t(ret);
        return HandleError::OK;
    } else {
        DWORD nread;
        auto ret = ReadFile(
          castToHandle(h._os.handle), data, DWORD(sz), &nread, nullptr);
        sz = 0;
        if (ret == FALSE)
            return getLastHandleError();
        if (nread == 0)
            return HandleError::EOF;
        sz = size_t(nread);
        return HandleError::OK;
    }
}

HandleError
write(Handle &h, size_t &sz, const char *data)
{
    ASSERT(h);
    if (h._os.is_socket) {
        auto ret = send(castToSocket(h._os.handle), data, int(sz), 0);
        sz = 0;
        if (ret == SOCKET_ERROR)
            return HandleError(getLastSocketError());
        sz = size_t(ret);
        return HandleError::OK;
    } else {
        DWORD nwrit;
        auto ret = WriteFile(
          castToHandle(h._os.handle), data, DWORD(sz), &nwrit, nullptr);
        sz = 0;
        if (ret == FALSE)
            return getLastHandleError();
        sz = size_t(nwrit);
        return HandleError::OK;
    }
}

HandleError
close(Handle &h)
{
    ASSERT(h);
    auto hndl = castToHandle(h._os.handle);
    h._os.handle = nullptr;
    h._mode = {};
    h._os.is_socket = false;
    if (CloseHandle(hndl) != FALSE)
        return HandleError::OK;
    else
        return HandleError::UNKNOWN;
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
    switch (proto) {
    case SP_TCP:
        type = SOCK_STREAM;
        break;
    default:
        ASSERT_FAIL();
    }

    SOCKET sock;
    sock = socket(PF_INET, type, 0);
    if (sock == INVALID_SOCKET) {
        err = getLastSocketError();
        return std::nullopt;
    }

    int enable = 1;
    auto ret = setsockopt(sock,
                          SOL_SOCKET,
                          SO_REUSEADDR,
                          reinterpret_cast<char *>(&enable),
                          sizeof enable);
    if (ret != 0)
        goto socket_err;

    {
        sockaddr_in server{};
        memset(&server, 0, sizeof server);
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = addr.addr4;
        server.sin_port = hton(port);

        ret = bind(sock, reinterpret_cast<sockaddr *>(&server), sizeof server);
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

    {
        Socket s;
        s._os.socket = castFromSocket(sock);
        err = SocketError::OK;
        return { std::move(s) };
    }

socket_err:
    err = getLastSocketError();
    closesocket(sock);
    return std::nullopt;
}

std::optional<Handle>
accept(Socket &s, SocketError &err)
{
    ASSERT(s);

    sockaddr_in client{};
    socklen_t clen = sizeof client;

    auto c = ::accept(
      castToSocket(s._os.socket), reinterpret_cast<sockaddr *>(&client), &clen);

    if (c == INVALID_SOCKET) {
        err = getLastSocketError();
        return std::nullopt;
    }

    Handle h;
    h._os.handle = castFromSocket(c);
    h._os.is_socket = true;
    h._mode = HM_READ | HM_WRITE;
    err = SocketError::OK;
    return { std::move(h) };
}

SocketError
close(Socket &sock)
{
    ASSERT(sock);
    auto hndl = castToHandle(sock._os.socket);
    sock._os.socket = nullptr;
    if (CloseHandle(hndl) != FALSE)
        return SocketError::OK;
    else
        return SocketError::UNKNOWN;
}

} // namespace io
} // namespace sys
