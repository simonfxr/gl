#ifndef MATH_MATH_HPP
#define MATH_MATH_HPP

#include "defs.h"

#if BUILD_SHARED_P
#    ifdef math_EXPORTS
#        define MATH_API SHARED_EXPORT
#    else
#        define MATH_API SHARED_IMPORT
#    endif
#    if (HU_COMP_CLANG_P || HU_COMP_INTEL_P) && !HU_COMP_MSVC_P &&             \
      !HU_OBJFMT_COFF_P
#        define MATH_EXTERN_TEMPLATE                                           \
            extern template __attribute__((visibility("default")))
#    endif
#else
#    define MATH_API
#endif

#ifndef MATH_EXTERN_TEMPLATE
#    define MATH_EXTERN_TEMPLATE extern template MATH_API
#endif

#endif
