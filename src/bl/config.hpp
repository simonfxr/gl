#ifndef BL_CONFIG_HPP
#define BL_CONFIG_HPP

#include "defs.h"

#include <hu/features.h>

#define BL_CAT PP_CAT
#define BL_TOSTR PP_TOSTR

#define BL_SELECT(P, A, B) BL_CAT(BL_SELECT_, P)(A, B)
#define BL_SELECT_0(A, B) B
#define BL_SELECT_1(A, B) A

#define BL_CONSTEXPR_IF(P) BL_SELECT(P, constexpr, /* empty */)

#define BL_inline HU_FORCE_INLINE inline
#define BL_restrict HU_RESTRICT

#if HU_COMP_MSVC_P
#    define BL_LIBC_CCONV __cdecl
#else
#    define BL_LIBC_CCONV
#endif

#endif
