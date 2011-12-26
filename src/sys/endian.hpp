#ifndef SYS_ENDIAN_HPP
#define SYS_ENDIAN_HPP

#include "defs.hpp"

#ifdef SYSTEM_UNIX
#include <arpa/inet.h>
#else
#error "no endian implementation available"
#endif

namespace sys {

using namespace defs;

inline uint32 hton(uint32 a) {
    return htonl(a);
}

inline uint16 hton(uint16 a) {
    return htons(a);
}

inline uint32 ntoh(uint32 a) {
    return ntohl(a);
}

inline uint16 ntoh(uint16 a) {
    return ntohs(a);
}

} // namespace sys

#endif
