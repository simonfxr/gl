#ifndef SYS_DEFS_HPP
#define SYS_DEFS_HPP

#include "defs.hpp"

#ifdef SYS_EXPORTS
#  define SYS_API SHARED_EXPORT
#else
#  define SYS_API SHARED_IMPORT
#endif

#endif
