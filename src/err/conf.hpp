#ifndef ERR_CONF_HPP
#define ERR_CONF_HPP

#include "defs.hpp"

#ifdef sys_EXPORTS
#define ERR_API SHARED_EXPORT
#else
#define ERR_API SHARED_IMPORT
#endif

#endif
