#ifndef UTIL_ENUM_FLAGS_HPP
#define UTIL_ENUM_FLAGS_HPP

#include "defs.h"

#include <type_traits>

#define DEF_ENUM_BIT_OP(ty, op)                                                \
    inline constexpr ty operator op(ty a, ty b)                                \
    {                                                                          \
        using T = std::underlying_type_t<ty>;                                  \
        return static_cast<ty>(static_cast<T>(a) op static_cast<T>(b));        \
    }                                                                          \
    inline constexpr ty operator PP_CAT(op, =)(ty &a, ty b)                    \
    {                                                                          \
        return a = a op b;                                                     \
    }

#define DEF_ENUM_BIT_OPS(ty)                                                   \
    DEF_ENUM_BIT_OP(ty, |)                                                     \
    DEF_ENUM_BIT_OP(ty, &)                                                     \
    DEF_ENUM_BIT_OP(ty, ^)                                                     \
    inline constexpr ty operator~(ty a)                                        \
    {                                                                          \
        using T = std::underlying_type_t<ty>;                                  \
        return static_cast<ty>(~static_cast<T>(a));                            \
    }

#endif
