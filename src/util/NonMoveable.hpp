#ifndef UTIL_NON_MOVEABLE_HPP
#define UTIL_NON_MOVEABLE_HPP

#include "defs.h"

struct NonMoveable
{
    constexpr NonMoveable() = default;
    NonMoveable(NonMoveable &&) = delete;
    NonMoveable &operator=(NonMoveable &&) = delete;
};

#endif
