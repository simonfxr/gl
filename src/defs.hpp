#ifndef _DEFS_HPP_INCLUDED_
#define _DEFS_HPP_INCLUDED_

#define DEFS_BEGIN_NAMESPACE namespace defs {
#define DEFS_END_NAMESPACE }

#include "defs.h"

namespace defs {

template <typename IntT>
inline size ___check_size(IntT x) {
    size s = size(x);
#ifdef SIGNED_SIZE
    /* DEBUG_ASSERT(s >= 0); */
#endif
    return s;
}

template <typename SizeT>
inline uptr ___check_unsize(SizeT s) {
#ifdef SIGNED_SIZE

#endif
    return uptr(s);
}

template <typename SizeT>
inline SizeT ___assert_size(SizeT s) {
#ifdef SIGNED_SIZE
//    ASSERT(s >= 0);
#endif
    return s;
}

#define SIZE(x) ::defs::___check_size(x)
#define UNSIZE(x) ::defs::___check_unsize(x)
#define ASSERT_SIZE(x) ::defs::___assert_size(x)

#define DEF_ENUM_BITOR(ty) inline ty operator |(ty a, ty b) { return ty(int(a) | int(b)); }

} // namespace defs

#endif
