#ifndef BL_HASH_HPP
#define BL_HASH_HPP

#include "defs.h"

namespace bl {

#define DEF_INT_HASH(t)                                                        \
    template<>                                                                 \
    struct hash<t>                                                             \
    {                                                                          \
        inline constexpr size_t operator()(t x) const noexcept                 \
        {                                                                      \
            return size_t(x);                                                  \
        }                                                                      \
    }

template<typename T>
struct hash;

// FIXME: take proper care of 32bit platforms, consider better hash algos

DEF_INT_HASH(bool);
DEF_INT_HASH(char);
DEF_INT_HASH(signed char);
DEF_INT_HASH(unsigned char);
DEF_INT_HASH(short);
DEF_INT_HASH(unsigned short);
DEF_INT_HASH(int);
DEF_INT_HASH(unsigned int);
DEF_INT_HASH(long);
DEF_INT_HASH(unsigned long);
DEF_INT_HASH(long long);
DEF_INT_HASH(unsigned long long);

#define BL_DEF_HASH_DELEGATOR(T)                                               \
    struct hash<T>                                                             \
    {                                                                          \
        inline size_t operator()(const T &x) const { return x.hash(); }        \
    }

} // namespace bl

#endif
