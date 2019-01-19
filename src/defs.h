#ifndef DEFS_H_INCLUDED
#define DEFS_H_INCLUDED

#include <hu/macros.h>
#include <hu/platform.h>

#ifdef HU_OS_WINDOWS
#define WIN32_LEAND_AND_MEAN 1
#define VC_EXTRALEAN 1
#define NOMINMAX 1
#define NOGDI 1
#endif

#ifdef HU_COMP_CLANG
#define CASE_UNREACHABLE
#else
#define CASE_UNREACHABLE ASSERT_FAIL()
#endif

#ifdef __cplusplus
#include <cstdint>
#else
#include <stdint.h>
#endif

#ifdef __cplusplus
#include <cstddef>
#else
#include <stddef.h>
#endif

#ifdef BUILD_SHARED
#define SHARED_IMPORT HU_LIB_IMPORT
#define SHARED_EXPORT HU_LIB_EXPORT
#else
#define SHARED_IMPORT
#define SHARED_EXPORT
#endif

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof *(x))

#ifdef DEBUG
#define ON_DEBUG(x)                                                            \
    do {                                                                       \
        (x);                                                                   \
    } while (0)
#define DEBUG_DECL(x) x
#else
#define ON_DEBUG(x) ((void) 0)
#define DEBUG_DECL(x)
#endif

#define likely hu_likely
#define unlikely hu_unlikely

#define RESTRICT HU_RESTRICT

#define PP_CAT_I(a, ...) a##__VA_ARGS__
#define PP_CAT(a, ...) PP_CAT_I(a, __VA_ARGS__)

#define PP_CAT3(a, b, ...) PP_CAT(PP_CAT(a, b), __VA_ARGS__)

#define PP_TOSTR0(x) #x
#define PP_TOSTR(x) PP_TOSTR0(x)

#define UNUSED(x) ((void) (x))

#if HU_COMP_CLANG_P
#define IGNORE_RESULT(x) ((void) (x))
#else
#define IGNORE_RESULT_(id, x)                                                  \
    do {                                                                       \
        auto PP_CAT(ignored_, id) = (x);                                       \
        UNUSED(PP_CAT(ignored_, id));                                          \
    } while (0)
#ifdef __COUNTER__
#define IGNORE_RESULT(x) IGNORE_RESULT_(__COUNTER__, x)
#else
#define IGNORE_RESULT(x) IGNORE_RESULT_(__LINE__, x)
#endif
#endif

#if HU_COMP_CLANG_P
#define PRAGMA_DIAGNOSTIC_PUSH _Pragma("clang diagnostic push")
#define PRAGMA_DIAGNOSTIC_POP _Pragma("clang diagnostic pop")
#define PRAGMA_DISABLE_DIAG_SWITCH                                             \
    _Pragma(PP_TOSTR(clang diagnostic ignored "-Wswitch"))                     \
      _Pragma(PP_TOSTR(clang diagnostic ignored "-Wswitch-enum"))
#define PRAGMA_DISABLE_DIAG_GLOBAL_DESTRUCTOR                                  \
    _Pragma(PP_TOSTR(clang diagnostic ignored "-Wexit-time-destructors"))      \
      _Pragma(PP_TOSTR(clang diagnostic ignored "-Wglobal-constructors"))
#define PRAGMA_DISABLE_DIAG_UNINITIALIZED                                      \
    _Pragma(PP_TOSTR(clang diagnostic ignored "-Wuninitialized"))
#define PRAGMA_DISABLE_DIAG_DISABLED_MACRO_EXPANSION                           \
    _Pragma(PP_TOSTR(clang diagnostic ignored "-Wdisabled-macro-expansion"))
#elif HU_COMP_GCC_P
#define PRAGMA_DIAGNOSTIC_PUSH _Pragma("GCC diagnostic push")
#define PRAGMA_DIAGNOSTIC_POP _Pragma("GCC diagnostic pop")
#define PRAGMA_DISABLE_DIAG_SWITCH                                             \
    _Pragma(PP_TOSTR(GCC diagnostic ignored "-Wswitch"))                       \
      _Pragma(PP_TOSTR(GCC diagnostic ignored "-Wswitch-enum"))
#define PRAGMA_DISABLE_DIAG_DISABLED_MACRO_EXPANSION
#define PRAGMA_DISABLE_DIAG_GLOBAL_DESTRUCTOR
#define PRAGMA_DISABLE_DIAG_UNINITIALIZED                                      \
    _Pragma(PP_TOSTR(GCC diagnostic ignored "-Wuninitialized"))
#define PRAGMA_DISABLE_DIAG_DISABLED_MACRO_EXPANSION
#else
#define PRAGMA_DIAGNOSTIC_PUSH
#define PRAGMA_DIAGNOSTIC_POP
#define PRAGMA_DISABLE_DIAG_SWITCH
#define PRAGMA_DISABLE_DIAG_GLOBAL_DESTRUCTOR
#define PRAGMA_DISABLE_DIAG_UNINITIALIZED
#define PRAGMA_DISABLE_DIAG_DISABLED_MACRO_EXPANSION
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

#endif
