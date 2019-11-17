#ifndef UTIL_NON_COPYABLE_HPP
#define UTIL_NON_COPYABLE_HPP

#include "defs.h"

struct NonCopyable
{
    constexpr NonCopyable() = default;
    NonCopyable(const NonCopyable &) = delete;
    NonCopyable &operator=(const NonCopyable &) = delete;
};

#endif
