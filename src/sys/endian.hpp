#ifndef SYS_ENDIAN_HPP
#define SYS_ENDIAN_HPP

#include "defs.hpp"

#ifdef SYSTEM_UNIX
#  include <arpa/inet.h>
#else
#  include "err/err.hpp"
#endif

namespace sys {

using namespace defs;
#ifdef SYSTEM_UNIX
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
#else
//assume host uses little endian
#define UNDEFINED ERR(std::string("not yet implemented"))

inline uint32 hton(uint32) {
    UNDEFINED;
    return 0;
}

inline uint16 hton(uint16) {
    UNDEFINED;
    return 0;
}

inline uint32 ntoh(uint32 a) {
    UNDEFINED;
    return 0;
}

inline uint16 ntoh(uint16 a) {
    UNDEFINED;
    return 0;
}
#undef UNDEFINED

#endif

} // namespace sys

#endif
