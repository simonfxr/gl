#include "sys/io.hpp"
#include "err/err.hpp"

namespace sys {

namespace io {

#define UNDEFINED ERR("not yet implemented"); return HE_UNKNOWN
#define UNDEFINED_S ERR("not yet implemented"); return SE_UNKNOWN

HandleError open(const std::string&, HandleMode, Handle *) {
    UNDEFINED;
}

HandleMode mode(Handle&) {
    UNDEFINED;
}

HandleError elevate(Handle&, HandleMode) {
    UNDEFINED;
}

HandleError read(Handle&, size&, char *) {
    UNDEFINED;
}

HandleError write(Handle&, size&, const char *) {
    UNDEFINED;
}

HandleError close(Handle&) {
    UNDEFINED;
}

SocketError listen(SocketProto, const IPAddr4&, uint16, SocketMode, Socket *) {
    UNDEFINED_S;
}

SocketError accept(Socket&, Handle *) {
    UNDEFINED_S;
}

SocketError close(Socket&) {
    UNDEFINED_S;
}

} // namespace io

} // namespace sys
