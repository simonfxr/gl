#ifndef SYS_IO_HPP
#define SYS_IO_HPP

#include "sys/conf.hpp"
#include "sys/endian.hpp"
#include "sys/io/Stream.hpp"

#include <string>


namespace sys {

namespace io {

using namespace defs;

struct Handle;
struct Socket;
struct IPAddr4;
struct HandleStream;

namespace {

const size HANDLE_READ_BUFFER_SIZE = 1024;
const size HANDLE_WRITE_BUFFER_SIZE = 1024;

typedef uint32 HandleMode;

const HandleMode HM_READ = 1;
const HandleMode HM_WRITE = 2;
const HandleMode HM_APPEND = 4;
const HandleMode HM_NONBLOCKING = 8;


typedef uint32 SocketProto;

const SocketProto SP_TCP = 1;


typedef uint32 SocketMode;

const SocketMode SM_NONBLOCKING = 1;

} // namespace anon

extern SYS_API const IPAddr4 IPA_ANY;
extern SYS_API const IPAddr4 IPA_LOCAL;

enum HandleError {
    HE_OK,
    HE_BLOCKED,
    HE_EOF,
    HE_BAD_HANDLE,
    HE_INVALID_PARAM,
    HE_UNKNOWN
};

struct SYS_API IPAddr4 {
    uint32 addr4; // bigendian/network byte order
    IPAddr4() {}
    IPAddr4(uint8 a, uint8 b, uint8 c, uint8 d)
        : addr4(hton((uint32(a) << 24) | (uint32(b) << 16) | (uint32(c) << 8) | uint32(d))) {}
        
    IPAddr4(uint32 addr) : addr4(addr) {}
};

} // namespace io

} // namespace sys

#ifdef SYSTEM_UNIX
#include "sys/io/io_unix.hpp"
#elif defined(SYSTEM_WINDOWS)
#include "sys/io/io_windows.hpp"
#else
#error "no IO implementation available"
#endif

namespace sys {

namespace io {

SYS_API HandleError open(const std::string&, HandleMode, Handle *);

SYS_API HandleMode mode(Handle&);

SYS_API HandleError elevate(Handle&, HandleMode);

SYS_API HandleError read(Handle&, size&, char *);

SYS_API HandleError write(Handle&, size&, const char *);

SYS_API HandleError close(Handle&);

enum SocketError {
    SE_OK,
    SE_BLOCKED,
    SE_BAD_SOCKET,
    SE_INVALID_PARAM,
    SE_UNKNOWN
};

SYS_API SocketError listen(SocketProto, const IPAddr4&, uint16, SocketMode, Socket *);

SYS_API SocketError accept(Socket&, Handle *);

SYS_API SocketError close(Socket&);

struct SYS_API HandleStream : public AbstractIOStream {
    Handle handle;
    char read_buffer[HANDLE_READ_BUFFER_SIZE];
    char write_buffer[HANDLE_WRITE_BUFFER_SIZE];
    defs::index read_cursor;
    defs::index write_cursor;

    HandleStream(const Handle& = Handle());
    ~HandleStream();

protected:
    void basic_close();
    StreamResult basic_flush();
    StreamResult basic_read(size&, char *);
    StreamResult basic_write(size&, const char *);
    StreamResult flush_buffer();
};

} // namespace io

} // namespace sys

#endif
