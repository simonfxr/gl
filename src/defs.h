#ifndef DEFS_H_INCLUDED
#define DEFS_H_INCLUDED

#define BL_DEBUG 1

#include <hu/macros.h>
#include <hu/os.h>

#ifdef HU_OS_WINDOWS
#    define WIN32_LEAND_AND_MEAN 1
#    define VC_EXTRALEAN 1
#    define NOMINMAX 1
#    define NOGDI 1
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
#    define SHARED_IMPORT HU_LIB_IMPORT
#    define SHARED_EXPORT HU_LIB_EXPORT
#else
#    define SHARED_IMPORT
#    define SHARED_EXPORT
#endif

#define FORCE_INLINE HU_FORCE_INLINE

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof *(x))

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

#define UNUSED(x) ((void) (x))

#if HU_COMP_CLANG_P
#    define IGNORE_RESULT(x) ((void) (x))
#else
#    define IGNORE_RESULT_(id, x)                                              \
        do {                                                                   \
            auto PP_CAT(ignored_, id) = (x);                                   \
            UNUSED(PP_CAT(ignored_, id));                                      \
        } while (0)
#    ifdef __COUNTER__
#        define IGNORE_RESULT(x) IGNORE_RESULT_(__COUNTER__, x)
#    else
#        define IGNORE_RESULT(x) IGNORE_RESULT_(__LINE__, x)
#    endif
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
#    define PRAGMA_DISABLE_DIAG_DEPRECATED_EX_SPEC                             \
        _Pragma(PP_TOSTR(clang diagnostic ignored                              \
                         "-Wdeprecated-dynamic-exception-spec"))
#    define PRAGMA_DISABLE_DIAG_RESERVED_ID_MACRO                              \
        _Pragma(PP_TOSTR(clang diagnostic ignored "-Wreserved-id-macro"))
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

#ifndef PRAGMA_DISABLE_DIAG_DEPRECATED_EX_SPEC
#    define PRAGMA_DISABLE_DIAG_DEPRECATED_EX_SPEC
#endif

#ifndef PRAGMA_DISABLE_DIAG_RESERVED_ID_MACRO
#    define PRAGMA_DISABLE_DIAG_RESERVED_ID_MACRO
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

#define BEGIN_NO_WARN_DEPRECATED_EX_SPEC                                       \
    PRAGMA_DIAGNOSTIC_PUSH PRAGMA_DISABLE_DIAG_DEPRECATED_EX_SPEC
#define END_NO_WARN_DEPRECATED_EX_SPEC PRAGMA_DIAGNOSTIC_POP

#define BEGIN_NO_WARN_RESERVED_ID_MACRO                                        \
    PRAGMA_DIAGNOSTIC_PUSH PRAGMA_DISABLE_DIAG_RESERVED_ID_MACRO
#define END_NO_WARN_RESERVED_ID_MACRO PRAGMA_DIAGNOSTIC_POP

#endif
