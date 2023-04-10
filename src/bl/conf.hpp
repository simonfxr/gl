#ifndef BL_CONF_HPP
#define BL_CONF_HPP

#include "defs.h"

#include <hu/os.h>

#ifdef bl_EXPORTS
#    define BL_API SHARED_EXPORT
#else
#    define BL_API SHARED_IMPORT
#endif

namespace bl {}

#endif // BL_CONF_HPP
