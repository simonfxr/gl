#ifndef BL_CONF_HPP
#define BL_CONF_HPP

#include <hu/annotations.h>
#include <hu/compiler.h>
#include <hu/macros.h>
#include <hu/objfmt.h>
#include <hu/os.h>

#if BUILD_SHARED_P
#    ifdef bl_EXPORTS
#        define BL_API HU_DSO_EXPORT
#    else
#        define BL_API HU_DSO_IMPORT
#    endif
#else
#    define BL_API
#endif

#define BL_inline HU_FORCE_INLINE inline
#define BL_wrapper HU_MACROLIKE
#define bl_likely hu_likely
#define bl_unlikely hu_unlikely

#define BL_PP_CAT_(a, ...) a##__VA_ARGS__
#define BL_PP_CAT(a, ...) BL_PP_CAT_(a, __VA_ARGS__)
#define BL_PP_TOSTR_(x) #x
#define BL_PP_TOSTR(x) BL_PP_TOSTR_(x)

#if HU_COMP_CLANG_P
#    define BL_DIAG_PUSH _Pragma("clang diagnostic push")
#    define BL_DIAG_POP _Pragma("clang diagnostic pop")
#    define BL_DIAG_IGN_GCC(...)
#    define BL_DIAG_IGN_CLANG(...)                                             \
        _Pragma(BL_PP_TOSTR(clang diagnostic ignored __VA_ARGS__))
#elif HU_COMP_GCC_P
#    define BL_DIAG_PUSH _Pragma("GCC diagnostic push")
#    define BL_DIAG_POP _Pragma("GCC diagnostic pop")
#    define BL_DIAG_IGN_GCC(...)                                               \
        _Pragma(BL_PP_TOSTR(GCC diagnostic ignored __VA_ARGS__))
#    define BL_DIAG_IGN_CLANG(...)
#else
#    define BL_DIAG_IGN_GCC(...)
#    define BL_DIAG_IGN_CLANG(...)
#    define BL_DIAG_PUSH
#    define BL_DIAG_POP
#endif

#if HU_COMP_CLANG_P
#    define BL_DIAG_IGN_GNUC(...) BL_DIAG_IGN_CLANG(__VA_ARGS__)
#else
#    define BL_DIAG_IGN_GNUC(...) BL_DIAG_IGN_GCC(__VA_ARGS__)
#endif

#define bl_declval(...) static_cast<__VA_ARGS__ (*)() noexcept>(nullptr)()

namespace bl {}

#endif // BL_CONF_HPP
