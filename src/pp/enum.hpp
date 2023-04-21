#ifndef PP_ENUM_SIMPLE_HPP
#define PP_ENUM_SIMPLE_HPP

#include "pp/basic.h"
#include "pp/map.h"

#define PP_ENUM_DEF_CONST1(...) 1

#define PP_ENUM_DEF_PLUS(...) +

#define PP_ENUM_DEF_TOSTR2(tag, str)                                           \
case tag:                                                                      \
    return ::bl::string_view(str, sizeof(str) - 1);

#define PP_ENUM_DEF_TOSTR1(tag) PP_ENUM_DEF_TOSTR2(tag, PP_TOSTR(tag))

#define PP_ENUM_DEF_WRAPPER(name) PP_CAT(name, _wrapper_type)

#define PP_DEF_ENUM(NAME, UNDERLYING, ...)                                     \
    struct PP_ENUM_DEF_WRAPPER(NAME)                                           \
    {                                                                          \
        using underlying_t = UNDERLYING;                                       \
        enum NAME : UNDERLYING                                                 \
        {                                                                      \
            __VA_ARGS__                                                        \
        };                                                                     \
        static inline constexpr underlying_t _count_value_ =                   \
          PP_MAP(PP_ENUM_DEF_CONST1, PP_ENUM_DEF_PLUS, __VA_ARGS__);           \
        friend constexpr auto invoke(::bl::enum_count_f,                       \
                                     ::bl::type_proxy<NAME>)                   \
        {                                                                      \
            return _count_value_;                                              \
        }                                                                      \
        friend constexpr ::bl::string_view to_string(NAME _enum_value_)        \
        {                                                                      \
            switch (_enum_value_) {                                            \
                PP_MAP(PP_ENUM_DEF_TOSTR1, PP_SEMI, __VA_ARGS__)               \
            }                                                                  \
            hu_unreachable();                                                  \
        }                                                                      \
                                                                               \
        template<typename OStream>                                             \
        friend constexpr decltype(auto) operator<<(OStream &out,               \
                                                   NAME _enum_value_)          \
        {                                                                      \
            return out << to_string(_enum_value_);                             \
        }                                                                      \
    };                                                                         \
    using NAME = PP_ENUM_DEF_WRAPPER(NAME)::NAME

#endif // PP_ENUM_SIMPLE_HPP
