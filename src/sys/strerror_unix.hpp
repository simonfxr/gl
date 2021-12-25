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
    strerror_r(errnum, str.data(), str.size());
    return str;
}

static inline auto
strerror_errno()
{
    return strerror_errno(errno);
}
} // namespace sys

#endif // SYS_STRERROR_UNIX_HPP
