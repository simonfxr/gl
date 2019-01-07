#ifndef DEFS_H_INCLUDED
#define DEFS_H_INCLUDED

#define SIGNED_SIZE

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

#define ARRAY_LENGTH(x) ::size_t(sizeof(x) / sizeof *(x))

#define UNUSED(x) ((void) (x))

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

#define CONCAT(a, b) CONCAT_AUX1(a, b)
#define CONCAT_AUX1(a, b) CONCAT_AUX2(a, b)
#define CONCAT_AUX2(a, b) CONCAT_AUX3(a, b)
#define CONCAT_AUX3(a, b) CONCAT_AUX4(a, b)
#define CONCAT_AUX4(a, b) CONCAT_AUX5(a, b)
#define CONCAT_AUX5(a, b) a##b

#define CONCAT3(a, b, c) CONCAT(a, CONCAT(b, c))

#define AS_STR(a) AS_STRING_AUX1(a)
#define AS_STRING(a) AS_STRING_AUX1(a)
#define AS_STRING_AUX1(a) AS_STRING_AUX2(a)
#define AS_STRING_AUX2(a) AS_STRING_AUX3(a)
#define AS_STRING_AUX3(a) AS_STRING_AUX4(a)
#define AS_STRING_AUX4(a) AS_STRING_AUX5(a)
#define AS_STRING_AUX5(a) AS_STRING_AUX6(a)
#define AS_STRING_AUX6(a) AS_STRING_AUX7(a)
#define AS_STRING_AUX7(a) #a

#if 0
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned uint32_t;
typedef unsigned long long uint64_t;

typedef signed char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long long int64_t;
#endif

#if HU_COMP_CLANG_P
#define PRAGMA_DIAGNOSTIC_PUSH _Pragma("clang diagnostic push")
#define PRAGMA_DIAGNOSTIC_POP _Pragma("clang diagnostic pop")
#define PRAGMA_DISABLE_DIAG_SWITCH                                             \
    _Pragma(AS_STR(clang diagnostic ignored "-Wswitch"))                       \
      _Pragma(AS_STR(clang diagnostic ignored "-Wswitch-enum"))
#define PRAGMA_DISABLE_DIAG_GLOBAL_DESTRUCTOR                                  \
    _Pragma(AS_STR(clang diagnostic ignored "-Wexit-time-destructors"))        \
      _Pragma(AS_STR(clang diagnostic ignored "-Wglobal-constructors"))
#define PRAGMA_DISABLE_DIAG_UNINITIALIZED                                      \
    _Pragma(AS_STR(clang diagnostic ignored "-Wuninitialized"))
#define PRAGMA_DISABLE_DIAG_DISABLED_MACRO_EXPANSION                           \
    _Pragma(AS_STR(clang diagnostic ignored "-Wdisabled-macro-expansion"))
#elif HU_COMP_GCC_P
#define PRAGMA_DIAGNOSTIC_PUSH _Pragma("GCC diagnostic push")
#define PRAGMA_DIAGNOSTIC_POP _Pragma("GCC diagnostic pop")
#define PRAGMA_DISABLE_DIAG_SWITCH                                             \
    _Pragma(AS_STR(GCC diagnostic ignored "-Wswitch"))                         \
      _Pragma(AS_STR(GCC diagnostic ignored "-Wswitch-enum"))
#define PRAGMA_DISABLE_DIAG_DISABLED_MACRO_EXPANSION
#define PRAGMA_DISABLE_DIAG_GLOBAL_DESTRUCTOR
#define PRAGMA_DISABLE_DIAG_UNINITIALIZED                                      \
    _Pragma(AS_STR(GCC diagnostic ignored "-Wuninitialized"))
#define PRAGMA_DISABLE_DIAG_DISABLED_MACRO_EXPANSION
#else
#define PRAGMA_DIAGNOSTIC_PUSH
#define PRAGMA_DIAGNOSTIC_POP
#define PARGMA_DISABLE_DIAG_SWITCH
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
