#ifndef DEFS_H_INCLUDED
#define DEFS_H_INCLUDED

#if defined __unix__ || defined __unix || defined _POSIX_VERSION ||            \
  defined __APPLE__
#    ifndef _GNU_SOURCE
#        ifndef _POSIX_C_SOURCE
#            define _POSIX_C_SOURCE 200809L
#        endif
#    endif
#endif

#include <hu/os.h>

#if HU_OS_WINDOWS_P
#    ifndef _WINVER
/* Windows 7 */
#        define _WINVER 0x0601
#    endif
#    undef _WIN32_WINNT
#    define _WIN32_WINNT _WINVER
#endif

#include <hu/hu.h>

#ifdef HU_OS_WINDOWS
#    define WIN32_LEAND_AND_MEAN 1
#    define VC_EXTRALEAN 1
#    define NOMINMAX 1
#    define NOGDI 1
#endif

#include <config.h>

#if HU_COMP_GNUC_P
#    if defined __OPTIMIZE__ && !BUILD_OPTIMIZED_P
#        error "__OPTIMIZE__ is defined but not BUILD_OPTIMIZED_P is false"
#    elif !defined __OPTIMIZE__ && BUILD_OPTIMIZED_P
#        error "__OPTIMIZE__ is undefined but BUILD_OPTIMIZED_P is true"
#    endif
#endif

#ifdef __cplusplus
#    include <cstdint>
#else
#    include <stdint.h>
#endif

#ifdef __cplusplus
#    include <cstddef>
#else
#    include <stddef.h>
#endif

#if BUILD_SHARED_P
#    define SHARED_IMPORT HU_DSO_IMPORT
#    define SHARED_EXPORT HU_DSO_EXPORT
#else
#    define SHARED_IMPORT
#    define SHARED_EXPORT
#endif

#ifndef NDEBUG
#    define ON_DEBUG(...)                                                      \
        do {                                                                   \
            __VA_ARGS__;                                                       \
        } while (0)
#    define DEBUG_DECL(...) __VA_ARGS__;
#else
#    define ON_DEBUG(...) ((void) 0)
#    define DEBUG_DECL(...)
#endif

#define likely hu_likely
#define unlikely hu_unlikely

#define RESTRICT HU_RESTRICT

#define PP_CAT_I(a, ...) a##__VA_ARGS__
#define PP_CAT(a, ...) PP_CAT_I(a, __VA_ARGS__)

#define PP_CAT3(a, b, ...) PP_CAT(PP_CAT(a, b), __VA_ARGS__)

#define PP_TOSTR0(x) #x
#define PP_TOSTR(x) PP_TOSTR0(x)

#define PP_ARG1(_1, ...) _1
#define PP_ARG2(_1, _2, ...) _2
#define PP_ARG3(_1, _2, _3, ...) _3
#define PP_ARG4(_1, _2, _3, _4, ...) _4
#define PP_ARG5(_1, _2, _3, _4, _5, ...) _5

#define PP_DROP1(_1, ...) __VA_ARGS__
#define PP_DROP2(_1, _2, ...) __VA_ARGS__
#define PP_DROP3(_1, _2, _3, ...) __VA_ARGS__
#define PP_DROP4(_1, _2, _3, _4, ...) __VA_ARGS__
#define PP_DROP5(_1, _2, _3, _4, _5, ...) __VA_ARGS__

#define UNUSED(...) ((void) (__VA_ARGS__))

#if HU_COMP_CLANG_P
#    define IGNORE_RESULT(x) ((void) (x))
#else
#    define IGNORE_RESULT(...)                                                 \
        ([](auto &&unused) { UNUSED(unused); }(__VA_ARGS__))
#endif

#if HU_COMP_CLANG_P
#    define PRAGMA_DIAGNOSTIC_PUSH _Pragma("clang diagnostic push")
#    define PRAGMA_DIAGNOSTIC_POP _Pragma("clang diagnostic pop")
#    define PRAGMA_DISABLE_DIAG_SWITCH                                         \
        _Pragma(PP_TOSTR(clang diagnostic ignored "-Wswitch"))                 \
          _Pragma(PP_TOSTR(clang diagnostic ignored "-Wswitch-enum"))
#    define PRAGMA_DISABLE_DIAG_GLOBAL_DESTRUCTOR                              \
        _Pragma(PP_TOSTR(clang diagnostic ignored "-Wexit-time-destructors"))  \
          _Pragma(PP_TOSTR(clang diagnostic ignored "-Wglobal-constructors"))
#    define PRAGMA_DISABLE_DIAG_UNINITIALIZED                                  \
        _Pragma(PP_TOSTR(clang diagnostic ignored "-Wuninitialized"))
#    define PRAGMA_DISABLE_DIAG_DISABLED_MACRO_EXPANSION                       \
        _Pragma(PP_TOSTR(clang diagnostic ignored "-Wdisabled-macro-"          \
                                                  "expansion"))
#    define PRAGMA_DISABLE_DIAG_MISMATCHED_TAGS                                \
        _Pragma(PP_TOSTR(clang diagnostic ignored "-Wmismatched-tags"))
#elif HU_COMP_GCC_P
#    define PRAGMA_DIAGNOSTIC_PUSH _Pragma("GCC diagnostic push")
#    define PRAGMA_DIAGNOSTIC_POP _Pragma("GCC diagnostic pop")
#    define PRAGMA_DISABLE_DIAG_SWITCH                                         \
        _Pragma(PP_TOSTR(GCC diagnostic ignored "-Wswitch"))                   \
          _Pragma(PP_TOSTR(GCC diagnostic ignored "-Wswitch-enum"))
#    define PRAGMA_DISABLE_DIAG_UNINITIALIZED                                  \
        _Pragma(PP_TOSTR(GCC diagnostic ignored "-Wuninitialized"))
#endif

#ifndef PRAGMA_DIAGNOSTIC_PUSH
#    define PRAGMA_DIAGNOSTIC_PUSH
#endif

#ifndef PRAGMA_DIAGNOSTIC_POP
#    define PRAGMA_DIAGNOSTIC_POP
#endif

#ifndef PRAGMA_DISABLE_DIAG_SWITCH
#    define PRAGMA_DISABLE_DIAG_SWITCH
#endif

#ifndef PRAGMA_DISABLE_DIAG_GLOBAL_DESTRUCTOR
#    define PRAGMA_DISABLE_DIAG_GLOBAL_DESTRUCTOR
#endif

#ifndef PRAGMA_DISABLE_DIAG_UNINITIALIZED
#    define PRAGMA_DISABLE_DIAG_UNINITIALIZED
#endif

#ifndef PRAGMA_DISABLE_DIAG_DISABLED_MACRO_EXPANSION
#    define PRAGMA_DISABLE_DIAG_DISABLED_MACRO_EXPANSION
#endif

#ifndef PRAGMA_DISABLE_DIAG_MISMATCHED_TAGS
#    define PRAGMA_DISABLE_DIAG_MISMATCHED_TAGS
#endif

#define BEGIN_NO_WARN_SWITCH PRAGMA_DIAGNOSTIC_PUSH PRAGMA_DISABLE_DIAG_SWITCH
#define END_NO_WARN_SWITCH PRAGMA_DIAGNOSTIC_POP

#define BEGIN_NO_WARN_GLOBAL_DESTRUCTOR                                        \
    PRAGMA_DIAGNOSTIC_PUSH PRAGMA_DISABLE_DIAG_GLOBAL_DESTRUCTOR
#define END_NO_WARN_GLOBAL_DESTRUCTOR PRAGMA_DIAGNOSTIC_POP

#define BEGIN_NO_WARN_UNINITIALIZED                                            \
    PRAGMA_DIAGNOSTIC_PUSH PRAGMA_DISABLE_DIAG_UNINITIALIZED
#define END_NO_WARN_UNINITIALIZED PRAGMA_DIAGNOSTIC_POP

#define BEGIN_NO_WARN_DISABLED_MACRO_EXPANSION                                 \
    PRAGMA_DIAGNOSTIC_PUSH PRAGMA_DISABLE_DIAG_DISABLED_MACRO_EXPANSION
#define END_NO_WARN_DISABLED_MACRO_EXPANSION PRAGMA_DIAGNOSTIC_POP

#define BEGIN_NO_WARN_MISMATCHED_TAGS                                          \
    PRAGMA_DIAGNOSTIC_PUSH PRAGMA_DISABLE_DIAG_MISMATCHED_TAGS
#define END_NO_WARN_MISMATCHED_TAGS PRAGMA_DIAGNOSTIC_POP

#endif
