#ifndef SYS_STRERROR_UNIX_HPP
#define SYS_STRERROR_UNIX_HPP

#include "defs.h"

#include <bl/array.hpp>
#include <bl/core.hpp>
#include <bl/string_view.hpp>

#include <cerrno>
#include <cstring>

namespace sys {

struct strerror_buf
{
    bl::array<char, 128> d{};

    operator bl::string_view() { return { d.data(), strlen(d.data()) }; }
};

static inline auto
strerror_errno(int errnum)
{
    strerror_buf s;
    auto ret = strerror_r(errnum, s.d.data(), s.d.size());
    auto okay = false;
    if constexpr (std::is_pointer_v<decltype(ret)>) {
        // GNU version: returns char *
        okay = !!ret;
        if (ret && ret != s.d.data()) {
            strncpy(s.d.data(), ret, sizeof(s.d) - 1);
            s.d[sizeof(s.d) - 1] = '\0';
        }
    } else {
        // XSI version: returns an int, 0 means success
        okay = !ret;
    }
    if (!okay) {
        decltype(s.d) retstr = { "Unknown error" };
        return strerror_buf{ retstr };
    }
    return s;
}

static inline auto
strerror_errno()
{
    return strerror_errno(bl::exchange(errno, 0));
}
} // namespace sys

#endif // SYS_STRERROR_UNIX_HPP
