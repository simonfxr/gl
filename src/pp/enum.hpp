#ifndef PP_ENUM_HPP
#define PP_ENUM_HPP

#include "err/err.hpp"
#include "pp/map.h"

#define ENUM_TOSTRING_CASE(ty, val)                                            \
    case ty::val:                                                              \
        return PP_TOSTR(val)

#define DEF_ENUM_CLASS(API, ty, underlying, ...)                               \
    enum class ty : underlying                                                 \
    {                                                                          \
        __VA_ARGS__                                                            \
    };                                                                         \
    inline const char *to_string_impl_(ty enum_val_) noexcept                  \
    {                                                                          \
        switch (enum_val_) {                                                   \
            PP_MAP_WITH_ARG(ENUM_TOSTRING_CASE, PP_SEMI, ty, __VA_ARGS__);     \
        }                                                                      \
        return nullptr;                                                        \
    }                                                                          \
    API const char *to_string(ty enum_val_) noexcept;                          \
    template<typename OStream>                                                 \
    OStream &operator<<(OStream &out, ty enum_val_)                            \
    {                                                                          \
        auto str = to_string(enum_val_);                                       \
        if (!str)                                                              \
            return out << PP_TOSTR(ty) << "("                                  \
                       << static_cast<underlying>(enum_val_) << ")";           \
        return out << str;                                                     \
    }

#define DEF_ENUM_CLASS_OPS(ty)                                                 \
    const char *to_string(ty enum_val_) noexcept                               \
    {                                                                          \
        return to_string_impl_(enum_val_);                                     \
    }

#endif
