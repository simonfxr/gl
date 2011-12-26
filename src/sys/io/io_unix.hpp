#ifndef SYS_IO_UNIX_HPP
#define SYS_IO_UNIX_HPP

namespace sys {

namespace io {

using namespace defs;

typedef int FileDescriptor;

struct Handle {
    HandleMode mode;
    FileDescriptor fd;
};

struct Socket {
    FileDescriptor socket;
};

struct IPAddr {
    uint32 addr4; // bigendian/network byte order
    IPAddr() {}
    IPAddr(uint8 a, uint8 b, uint8 c, uint8 d)
        : addr4(hton((uint32(a) << 24) | (uint32(b) << 16) | (uint32(c) << 8) | uint32(d))) {}
        
    IPAddr(uint32 addr) : addr4(addr) {}
};

} // namespace io

} // namespace sys


#endif
