#ifndef SYS_DEFS_HPP
#define SYS_DEFS_HPP

#include "defs.h"
#include <hu/os.h>

#ifdef sys_EXPORTS
#    define SYS_API SHARED_EXPORT
#else
#    define SYS_API SHARED_IMPORT
#endif

#endif
