#ifndef GLT_CONF_HPP
#define GLT_CONF_HPP

#include "defs.h"

#ifdef glt_EXPORTS
#    define GLT_API SHARED_EXPORT
#else
#    define GLT_API SHARED_IMPORT
#endif

#endif
