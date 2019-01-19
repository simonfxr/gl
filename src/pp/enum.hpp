#ifndef PP_ENUM_HPP
#define PP_ENUM_HPP

#include "util/Comparable.hpp"
#include "pp/basic.h"

#define PP_ENUM_DEF_VALLIST_V0(val) val
#define PP_ENUM_DEF_VALLIST_V(val) , val

#define PP_ENUM_DEF_VALLIST(DEFN)                                              \
    DEFN(PP_ARG3, PP_ENUM_DEF_VALLIST_V0, PP_ENUM_DEF_VALLIST_V)

#define PP_ENUM_DEF_NAME(DEFN) DEFN(PP_ARG1, PP_IGNORE, PP_IGNORE)

#define PP_ENUM_DEF_UNDERLYING(DEFN) DEFN(PP_ARG2, PP_IGNORE, PP_IGNORE)

#define PP_ENUM_DEF_VALUE_COUNT_V0(_) 1
#define PP_ENUM_DEF_VALUE_COUNT_V(_) +1

#define PP_ENUM_DEF_VALUE_COUNT(DEFN)                                          \
    DEFN(PP_ARG3, PP_ENUM_DEF_VALUE_COUNT_V0, PP_ENUM_DEF_VALUE_COUNT_V)

#define PP_DEF_ENUM_WITH_API(API, DEFN)                                        \
    struct API PP_ENUM_DEF_NAME(DEFN) : Comparable<PP_ENUM_DEF_NAME(DEFN)>     \
    {                                                                          \
        using underlying_t = PP_ENUM_DEF_UNDERLYING(DEFN);                     \
        using enum_tag_t = void; /* tag used by std::enable_if<...> */         \
        enum enum_t : underlying_t                                             \
        {                                                                      \
            PP_ENUM_DEF_VALLIST(DEFN)                                          \
        };                                                                     \
        enum_t value{};                                                        \
        static inline constexpr underlying_t count =                           \
          PP_ENUM_DEF_VALUE_COUNT(DEFN);                                       \
        constexpr PP_ENUM_DEF_NAME(DEFN)() = default;                          \
        constexpr PP_ENUM_DEF_NAME(DEFN)(enum_t v) : value(v) {}               \
        constexpr explicit PP_ENUM_DEF_NAME(DEFN)(underlying_t v)              \
          : value(enum_t(v))                                                   \
        {}                                                                     \
        const char *to_string() const;                                         \
                                                                               \
        constexpr underlying_t numeric() const { return underlying_t(value); } \
                                                                               \
        template<typename OStream>                                             \
        friend OStream &operator<<(OStream &out,                               \
                                   PP_ENUM_DEF_NAME(DEFN) enum_val)            \
        {                                                                      \
            return out << enum_val.to_string();                                \
        }                                                                      \
                                                                               \
        template<typename OStream>                                             \
        friend OStream &operator<<(OStream &out, enum_t enum_val)              \
        {                                                                      \
            return out << PP_ENUM_DEF_NAME(DEFN)(enum_val);                    \
        }                                                                      \
                                                                               \
        friend inline const char *to_string(PP_ENUM_DEF_NAME(DEFN) enum_val)   \
        {                                                                      \
            return enum_val.to_string();                                       \
        }                                                                      \
                                                                               \
        friend inline const char *to_string(                                   \
          PP_ENUM_DEF_NAME(DEFN)::enum_t enum_val)                             \
        {                                                                      \
            return PP_ENUM_DEF_NAME(DEFN)(enum_val).to_string();               \
        }                                                                      \
                                                                               \
        constexpr bool is_valid()                                              \
        {                                                                      \
            return underlying_t{} <= underlying_t(value) &&                    \
                   size_t(underlying_t(value)) < count;                        \
        }                                                                      \
                                                                               \
        friend constexpr bool operator==(PP_ENUM_DEF_NAME(DEFN) x,             \
                                         PP_ENUM_DEF_NAME(DEFN) y)             \
        {                                                                      \
            return x.value == y.value;                                         \
        }                                                                      \
                                                                               \
        friend constexpr bool operator!=(PP_ENUM_DEF_NAME(DEFN) x,             \
                                         PP_ENUM_DEF_NAME(DEFN) y)             \
        {                                                                      \
            return x.value != y.value;                                         \
        }                                                                      \
                                                                               \
        friend constexpr int compare(PP_ENUM_DEF_NAME(DEFN) x,                 \
                                     PP_ENUM_DEF_NAME(DEFN) y)                 \
        {                                                                      \
            return underlying_t(x.value) < underlying_t(y.value)               \
                     ? -1                                                      \
                     : underlying_t(x.value) > underlying_t(y.value) ? 1 : 0;  \
        }                                                                      \
    }

#define PP_DEF_ENUM(DEFN) PP_DEF_ENUM_WITH_API(PP_NIL, DEFN)

#define PP_DEF_ENUM_TO_STRING_V(c)                                             \
    case c:                                                                    \
        return PP_TOSTR(c);

#define PP_DEF_ENUM_IMPL(DEFN)                                                 \
    const char *PP_ENUM_DEF_NAME(DEFN)::to_string() const                      \
    {                                                                          \
        switch (value) {                                                       \
            DEFN(PP_ARG3, PP_DEF_ENUM_TO_STRING_V, PP_DEF_ENUM_TO_STRING_V)    \
        }                                                                      \
        UNREACHABLE;                                                      \
    }

#endif
