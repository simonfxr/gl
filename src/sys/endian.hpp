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

#define UNDEFINED ::err::fatalError(_CURRENT_LOCATION, ::err::FatalError, ::err::ErrorArgs(sys::io::stdout(), "not yet implemented"))

inline uint32 hton(uint32 x) {
//    UNDEFINED;
    // assume little endian
#define BYTE(x, i, j) (((x >> (8 * i)) & 0xFF) << (8 * j))
    return BYTE(x, 0, 3) | BYTE(x, 1, 2) | BYTE(x, 2, 1) | BYTE(x, 3, 0);
#undef BYTE
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
