#ifndef UTIL_NON_COPYMOVE_HPP
#define UTIL_NON_COPYMOVE_HPP

#include "defs.h"

#define DISABLE_COPY_MEMBERS(T)                                                \
    T(const T &) = delete;                                                     \
    T &operator=(const T &) = delete

struct NonCopyable
{
    constexpr NonCopyable() = default;
    DISABLE_COPY_MEMBERS(NonCopyable);
};

#define DISABLE_MOVE_MEMBERS(T)                                                \
    T(T &&) = delete;                                                          \
    T &operator=(T &&) = delete

struct NonMoveable
{
    constexpr NonMoveable() = default;
    DISABLE_MOVE_MEMBERS(NonMoveable);
};

#define DISABLE_COPY_MOVE_MEMBERS(T)                                           \
    DISABLE_COPY_MEMBERS(T);                                                   \
    DISABLE_MOVE_MEMBERS(T)

struct NonCopyMoveable
  : private NonCopyable
  , private NonMoveable
{
    constexpr NonCopyMoveable() = default;
    DISABLE_COPY_MOVE_MEMBERS(NonCopyMoveable);
};

#endif
