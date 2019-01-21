#ifndef MATH_MATH_HPP
#define MATH_MATH_HPP

#include "defs.h"

#if BUILD_SHARED_P
#    ifdef math_EXPORTS
#        define MATH_API SHARED_EXPORT
#    else
#        define MATH_API SHARED_IMPORT
#    endif
#else
#    define MATH_API
#endif

#endif
