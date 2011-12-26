#ifndef SYS_IO_HPP
#define SYS_IO_HPP

#include "defs.hpp"
#include "sys/endian.hpp"

#include <string>

namespace sys {

namespace io {

using namespace defs;

struct Handle;
struct Socket;
struct IPAddr;

namespace {

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

} // namespace io

} // namespace sys

#ifdef SYSTEM_UNIX
#include "sys/io/io_unix.hpp"
#else
#error "no IO implementation available"
#endif

#endif
