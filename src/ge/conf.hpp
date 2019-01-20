#ifndef GE_CONF_HPP
#define GE_CONF_HPP

#include "defs.h"

#ifdef ge_EXPORTS
#    define GE_API SHARED_EXPORT
#else
#    define GE_API SHARED_IMPORT
#endif

#endif
