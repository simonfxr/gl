#ifndef UTIL_COMPARABLE_HPP
#define UTIL_COMPARABLE_HPP

#include "defs.h"

#include <type_traits>
#include <utility>

template<typename T>
std::enable_if_t<std::is_arithmetic_v<T>, int>
compare(const T &a, const T &b)
{
    return a < b ? -1 : a > b ? 1 : 0;
}

template<typename T>
std::enable_if_t<std::is_enum_v<T>, int>
compare(const T &a, const T &b)
{
    using U = std::underlying_type_t<T>;
    return compare(static_cast<U>(a), static_cast<U>(b));
}

template<typename T>
struct Comparable
{};

#define DEF_COMPARABLE_REL_OP(op)                                              \
    template<typename T>                                                       \
    bool operator op(const Comparable<T> &a, const Comparable<T> &b)           \
    {                                                                          \
        return compare(static_cast<const T &>(a), static_cast<const T &>(b))   \
          op 0;                                                                \
    }

DEF_COMPARABLE_REL_OP(==)
DEF_COMPARABLE_REL_OP(!=)
DEF_COMPARABLE_REL_OP(<)
DEF_COMPARABLE_REL_OP(<=)
DEF_COMPARABLE_REL_OP(>)
DEF_COMPARABLE_REL_OP(>=)

#undef DEF_COMPARABLE_REL_OP

inline int
chained_compare()
{
    return 0;
}

template<typename F, typename... Fs>
int
chained_compare(F &&f, Fs &&...fs)
{
    int ret = std::forward<F>(f).operator()();
    if (ret != 0)
        return ret;
    return chained_compare(std::forward<Fs>(fs)...);
}

#endif
