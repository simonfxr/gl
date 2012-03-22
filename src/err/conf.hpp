#ifndef ERR_DEFS_HPP
#define ERR_DEFS_HPP

#include "defs.hpp"

#if defined(SYS_EXPORTS)
#  define ERR_API SHARED_EXPORT
#else
#  define ERR_API SHARED_IMPORT
#endif

#endif
