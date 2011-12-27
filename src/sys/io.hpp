#ifndef SYS_IO_HPP
#define SYS_IO_HPP

#include "defs.hpp"
#include "sys/endian.hpp"
#include "sys/io/Stream.hpp"

#include <string>


namespace sys {

namespace io {

using namespace defs;

struct Handle;
struct Socket;
struct IPAddr;
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

extern const IPAddr IPA_ANY;
extern const IPAddr IPA_LOCAL;

enum HandleError {
    HE_OK,
    HE_BLOCKED,
    HE_EOF,
    HE_BAD_HANDLE,
    HE_INVALID_PARAM,
    HE_UNKNOWN
};

} // namespace io

} // namespace sys

#ifdef SYSTEM_UNIX
#include "sys/io/io_unix.hpp"
#else
#error "no IO implementation available"
#endif

namespace sys {

namespace io {

HandleError open(const std::string&, HandleMode, Handle *);

HandleMode mode(Handle&);

HandleError elevate(Handle&, HandleMode);

HandleError read(Handle&, size&, char *);

HandleError write(Handle&, size&, const char *);

HandleError close(Handle&);

enum SocketError {
    SE_OK,
    SE_BLOCKED,
    SE_BAD_SOCKET,
    SE_INVALID_PARAM,
    SE_UNKNOWN
};

SocketError listen(SocketProto, const IPAddr&, uint16, SocketMode, Socket *);

SocketError accept(Socket&, Handle *);

SocketError close(Socket&);

struct HandleStream : public AbstractIOStream {
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
