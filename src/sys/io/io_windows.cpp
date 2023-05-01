#include "sys/io.hpp"

#include "err/err.hpp"
#include "sys/module.hpp"
#include "sys/win_utf_conv.hpp"

#include <cassert>
#include <cstdio>
#include <exception>

#include <ws2tcpip.h>

#include <winsock2.h>

#include <windows.h>

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

NativeHandle
castFromHandle(HANDLE h)
{
    return reinterpret_cast<void *>(h);
}

HANDLE
castToHandle(NativeHandle)
{
    return reinterpret_cast<HANDLE>(h);
}

IOError
getLastIOError()
{
    auto err = GetLastError();
    switch (err) {
    case ERROR_HANDLE_EOF:
        return IOError::EOF;
    case ERROR_INVALID_HANDLE:
        return IOError::BAD_HANDLE;
    default:
        return IOError::UNKNOWN;
    }
}

IOError
convertSocketError(DWORD err)
{
    switch (err) {
    case WSAEWOULDBLOCK:
        return IOError::BLOCKED;
    case WSAECONNRESET:
        return IOError::EOF;
    case WSAENOTSOCK:
    case WSAEBADF:
        return IOError::BAD_HANDLE;
    default:
        return IOError::UNKNOWN;
    }
}

IOError
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
stdin_handle(){
    return Handle::fromNative(castFromHandle(GetStdHandle(STD_INPUT_HANDLE)))
}

SYS_API Handle stdout_handle()
{
    return Handle::fromNative(castFromHandle(GetStdHandle(STD_OUTPUT_HANDLE)));
}

SYS_API Handle
stderr_handle()
{
    return Handle::fromNative(castFromHandle(GetStdHandle(STD_ERROR_HANDLE)));
}

IOResult<Handle>
open(bl::string_view path, HandleMode mode)
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
    if (hndl == INVALID_HANDLE_VALUE)
        return util::unexpected{ getLastIOError() };
    return Handle::fromNative(castFromHandle(hndl), mode);
    return { bl::move(h) };
}

IOError
elevate(Handle &h, HandleMode hm)
{
    ASSERT(h);
    auto do_shutdown = false;
    auto mode = h.mode();
    int sd_mode = 0;
    if ((mode & HM_READ) != (hm & HM_READ)) {
        if (hm & HM_READ)
            return IOError::UNKNOWN;
        do_shutdown = true;
        sd_mode = SD_RECEIVE;
    }
    if ((mode & HM_WRITE) != (hm & HM_WRITE)) {
        if (hm & HM_WRITE)
            return IOError::UNKNOWN;
        sd_mode = do_shutdown && sd_mode == SD_RECEIVE ? SD_BOTH : SD_SEND;
        do_shutdown = true;
    }
    if (do_shutdown) {
        if (shutdown(castToSocket(h.native()), sd_mode) == SOCKET_ERROR) {
            return getLastSocketError();
        }
    }

    mode &= ~(HM_READ | HM_WRITE);
    mode |= (hm & (HM_READ | HM_WRITE));

    if ((mode & HM_NONBLOCKING) != (hm & HM_NONBLOCKING)) {
        unsigned long val = !!(hm & HM_NONBLOCKING);
        if (ioctlsocket(castToSocket(h.native()), FIONBIO, &val) ==
            SOCKET_ERROR)
            return getLastSocketError();
        mode &= ~HM_NONBLOCKING;
        mode |= hm & HM_NONBLOCKING;
    }
    h = Handle::fromNative(bl::move(h).takeNative(), mode);
    return IOError::OK;
}

bl::pair<bl::usize, IOError>
tag_invoke(read_f, Handle &h, bl::mut_byte_span buf)
{
    ASSERT(h);
    if (buf.empty())
        return { 0, IOError::OK };
    DWORD nread;
    auto ret = ReadFile(
      castToHandle(h.native()), buf.data(), DWORD(buf.size()), &nread, nullptr);
    if (ret == FALSE)
        return { 0, getLastIOError() };
    if (nread == 0)
        return { 0, IOError::EOF };
    return { bl::usize(nread), IOError::OK };
}

bl::pair<bl::usize, IOError>
tag_invoke(write_f, Handle &h, bl::byte_span buf)
{
    ASSERT(h);
    DWORD nwrit;
    auto ret = WriteFile(
      castToHandle(h.native()), buf.data(), DWORD(buf.size()), &nwrit, nullptr);
    if (ret == FALSE)
        return { 0, getLastIOError() };
    return { bl::usize(nwrit), IOError::OK };
}

IOError
tag_invoke(close_f, Handle &h)
{
    ASSERT(h);
    auto hndl = castToHandle(bl::move(h.takeNative()));
    if (CloseHandle(hndl) != FALSE)
        return IOError::OK;
    else
        return IOError::UNKNOWN;
}

IOResult<Handle>
listen(SocketProto proto, const IPAddr4 &addr, uint16_t port, SocketMode mode)
{
    const int BACKLOG = 16;

    int type;
    switch (proto) {
    case SP_TCP:
        type = SOCK_STREAM;
        break;
    default:
        UNREACHABLE;
    }

    SOCKET sock;
    sock = socket(PF_INET, type, 0);
    if (sock == INVALID_SOCKET)
        return util::unexpected{ getLastSocketError() };

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

    return { Handle::fromNative(castFromSocket(sock)) };

socket_err:
    auto err = getLastSocketError();
    closesocket(sock);
    return util::unexpected{ err };
}

IOResult<Handle>
accept(Handle &s)
{
    ASSERT(s);

    sockaddr_in client{};
    socklen_t clen = sizeof client;

    auto c = ::accept(
      castToSocket(s.native()), reinterpret_cast<sockaddr *>(&client), &clen);

    if (c == INVALID_SOCKET)
        return util::unexpected{ getLastSocketError() };

    return { Handle::fromNative(castFromSocket(c), HM_READ | HM_WRITE) };
}

} // namespace io
} // namespace sys
