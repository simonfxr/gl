#include "sys/io.hpp"

#include "err/err.hpp"
#include "sys/win_utf_conv.hpp"

#include <Windows.h>

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

    if (mode & HM_NONBLOCKING) {
        WARN("HM_NONBLOCKING is ignored on files");
    }

    return m;
}

W32_HANDLE *
castHandle(HANDLE h)
{
    return reinterpret_cast<W32_HANDLE *>(h);
}

HANDLE
castHandle(W32_HANDLE *h)
{
    return reinterpret_cast<HANDLE>(h);
}

} // namespace

#define UNDEFINED                                                              \
    ERR("not yet implemented");                                                \
    return HE_UNKNOWN
#define UNDEFINED_S                                                            \
    ERR("not yet implemented");                                                \
    return SE_UNKNOWN

SYS_API Handle
stdin_handle()
{
    Handle h;
    h.is_handle = true;
    h.handle = castHandle(GetStdHandle(STD_INPUT_HANDLE));
    return h;
}

SYS_API Handle
stdout_handle()
{
    Handle h;
    h.is_handle = true;
    h.handle = castHandle(GetStdHandle(STD_OUTPUT_HANDLE));
    return h;
}

SYS_API Handle
stderr_handle()
{
    Handle h;
    h.is_handle = true;
    h.handle = castHandle(GetStdHandle(STD_ERROR_HANDLE));
    return h;
}

HandleError
open(const std::string &path, HandleMode mode, Handle *h)
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
        return HE_UNKNOWN;
    h->is_handle = true;
    h->handle = castHandle(hndl);
    return HE_OK;
}

HandleMode
mode(Handle &)
{
    UNDEFINED;
}

HandleError
elevate(Handle &, HandleMode)
{
    UNDEFINED;
}

HandleError
read(Handle &h, size_t &sz, char *data)
{
    if (sz == 0)
        return HE_OK;
    ASSERT(h.is_handle);
    DWORD nread;
    if (ReadFile(castHandle(h.handle), data, DWORD(sz), &nread, nullptr) !=
        FALSE) {
        sz = size_t(nread);
        if (sz == 0)
            return HE_EOF;
        return HE_OK;
    } else {
        sz = 0;
        if (GetLastError() == ERROR_HANDLE_EOF)
            return HE_EOF;
        return HE_UNKNOWN;
    }
}

HandleError
write(Handle &h, size_t &sz, const char *data)
{
    ASSERT(h.is_handle);
    DWORD nwrit;
    if (WriteFile(castHandle(h.handle), data, DWORD(sz), &nwrit, nullptr) !=
        FALSE) {
        sz = size_t(nwrit);
        return HE_OK;
    } else {
        sz = 0;
        return HE_UNKNOWN;
    }
}

HandleError
close(Handle &h)
{
    ASSERT(h.is_handle);
    if (CloseHandle(castHandle(h.handle)) != FALSE)
        return HE_OK;
    else
        return HE_UNKNOWN;
}

SocketError
listen(SocketProto, const IPAddr4 &, uint16_t, SocketMode, Socket *)
{
    UNDEFINED_S;
}

SocketError
accept(Socket &, Handle *)
{
    UNDEFINED_S;
}

SocketError
close(Socket &)
{
    UNDEFINED_S;
}

} // namespace io
} // namespace sys
