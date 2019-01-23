#ifndef BL_DEBUG_HPP
#define BL_DEBUG_HPP

#ifdef BL_DEBUG
#    define BL_constexpr
#    include "bl/string_view_fwd.hpp"
#    include "err/err.hpp"
#    define BL_ASSERT ASSERT
#else
#    define BL_constexpr constexpr
#    define BL_ASSERT(...)
#endif

#endif
