#ifndef BL_COMPARE_HPP
#define BL_COMPARE_HPP

#include "bl/core.hpp"
#include "bl/type_traits.hpp"

namespace bl {

template<typename T>
bl::enable_if_t<std::is_arithmetic_v<T>, int>
compare(const T &a, const T &b)
{
    return a < b ? -1 : a > b ? 1 : 0;
}

template<typename T>
bl::enable_if_t<std::is_enum_v<T>, int>
compare(const T &a, const T &b)
{
    using U = std::underlying_type_t<T>;
    return compare(static_cast<U>(a), static_cast<U>(b));
}

template<typename T>
struct comparable
{};

#define COMPARE_VIA_COMPARABLE                                                 \
    compare(static_cast<const T &>(a), static_cast<const T &>(b))

#define BL_DEF_REL_OP_VIA(pref, ta, tb, op, VIA)                               \
    pref bool operator op(ta a, tb b) { return VIA op 0; }

#define BL_DEF_REL_OPS_VIA(pref, ta, tb, VIA)                                  \
    BL_DEF_REL_OP_VIA(pref, ta, tb, ==, VIA)                                   \
    BL_DEF_REL_OP_VIA(pref, ta, tb, !=, VIA)                                   \
    BL_DEF_REL_OP_VIA(pref, ta, tb, <, VIA)                                    \
    BL_DEF_REL_OP_VIA(pref, ta, tb, <=, VIA)                                   \
    BL_DEF_REL_OP_VIA(pref, ta, tb, >, VIA)                                    \
    BL_DEF_REL_OP_VIA(pref, ta, tb, >=, VIA)

BL_DEF_REL_OPS_VIA(template<typename T> inline,
                   const comparable<T> &,
                   const comparable<T> &,
                   COMPARE_VIA_COMPARABLE)

#undef COMPARE_VIA_COMPARABLE

inline int
chained_compare()
{
    return 0;
}

template<typename F, typename... Fs>
inline int
chained_compare(F &&f, Fs &&... fs)
{
    int ret = bl::forward<F>(f).operator()();
    if (ret != 0)
        return ret;
    return chained_compare(bl::forward<Fs>(fs)...);
}

} // namespace bl
#endif
