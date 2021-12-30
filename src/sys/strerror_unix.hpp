#ifndef SYS_STRERROR_UNIX_HPP
#define SYS_STRERROR_UNIX_HPP

#include "defs.h"

#include <array>
#include <cerrno>
#include <cstring>

namespace sys {

static inline auto
strerror_errno(int errnum)
{
    errno = 0;
    std::array<char, 128> str{};
    auto ret = strerror_r(errnum, str.data(), str.size());
    auto okay = false;
    if constexpr (std::is_pointer_v<decltype(ret)>) {
        // GNU version: returns char *
        okay = !!ret;
        if (ret && ret != str.data()) {
            strncpy(str.data(), ret, sizeof(str) - 1);
            str[sizeof(str) - 1] = '\0';
        }
    } else {
        // XSI version: returns an int, 0 means success
        okay = !ret;
    }
    if (!okay) {
        decltype(str) retstr = { "Unknown error" };
        return retstr;
    }
    return str;
}

static inline auto
strerror_errno()
{
    return strerror_errno(errno);
}
} // namespace sys

#endif // SYS_STRERROR_UNIX_HPP
