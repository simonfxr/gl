#ifndef BL_DEBUG_HPP
#define BL_DEBUG_HPP

#ifdef BL_DEBUG
#    define BL_constexpr
#    define BL_ASSERT(...) ((void) 0)
#    define BL_UNREACHABLE BL_ASSERT(0 && "unreachable")
#else
#    define BL_constexpr constexpr
#    define BL_ASSERT(...) ((void) 0)
#    define BL_UNREACHABLE hu_assume_unreachable()
#endif

#endif
