#ifndef DEFS_H_INCLUDED
#define DEFS_H_INCLUDED

#define SIGNED_SIZE

#include <hu/macros.h>
#include <hu/platform.h>

#ifndef DEFS_BEGIN_NAMESPACE
#define DEFS_BEGIN_NAMESPACE
#define DEFS_END_NAMESPACE
#endif

#ifdef HU_OS_WINDOWS
#define WIN32_LEAND_AND_MEAN 1
#define VC_EXTRALEAN 1
#define NOMINMAX 1
#define NOGDI 1
#endif

#ifndef HU_COMP_MSVC
#ifdef __cplusplus
#include <cstdint>
#else
#include <stdint.h>
#endif
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

#ifdef HU_COMP_GNULIKE
#define HAVE_ALIGNOF_EXPR
#define HAVE_ALIGNOF_TYPE
#define ALIGNOF_EXPR(e) __alignof__(e)
#define ALIGNOF_TYPE(t) __alignof__(t)
#elif defined(HU_COMP_MSVC)
#define HAVE_ALIGNOF_TYPE
#define ALIGNOF_TYPE(t) __alignof(t)
#define ALIGNOF_EXPR(e) alignof_expr_not_defined
#else
#define HAVE_ALIGNOF_TYPE
#define ALIGNOF_TYPE(t)                                                        \
    offsetof(                                                                  \
      struct {                                                                 \
          char ___c;                                                           \
          t ___x;                                                              \
      },                                                                       \
      ___x)
#define ALIGNOF_EXPR(e) alignof_expr_not_defined
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

#ifdef HU_COMP_GNULIKE
#define ATTRS(...) __attribute__((__VA_ARGS__))
#elif defined(HU_COMP_MSVC)
#define ATTRS(...) __declspec(__VA_ARGS)
#else
#define ATTRS(...)
#endif

#ifdef HU_COMP_GNULIKE
#define ATTR_WARN_UNUSED warn_unused_result
#define ATTR_NO_WARN_UNUSED_DEF unused
#define ATTR_ALIGNED(n) aligned(n)
#define ATTR_NOINLINE noinline
#define ATTR_NOTHROW nothrow
#define ATTR_FORCE_INLINE always_inline
#define ATTR_NORETURN noreturn
#define ATTR_PACKED packed
#define PRAGMA_PUSH_IGNORE_EXIT_TIME_DESTRUCTOR                                \
    _Pragma("GCC diagnostic push")                                             \
      _Pragma("GCC diagnostic ignored \"-Wexit-time-destructors\"")            \
        _Pragma("GCC diagnostic ignored \"-Wglobal-constructors\"")

#define PRAGMA_POP _Pragma("GCC diagnostic pop")
#elif defined(HU_COMP_MSVC)
#define ATTR_WARN_UNUSED
#define ATTR_NO_WARN_UNUSED_DEF
#define ATTR_ALIGNED(n) align(n)
#define ATTR_NOINLINE noinline
#define ATTR_NOTHROW nothrow
#define ATTR_FORCE_INLINE
#define ATTR_NORETURN noreturn
#define ATTR_PACKED
#define PRAGMA_PUSH_IGNORE_EXIT_TIME_DESTRUCTOR
#define PRAGMA_POP
#endif

#define likely hu_likely
#define unlikely hu_unlikely

#ifdef GNU_EXTENSIONS
#define HAVE_THREAD_LOCAL
#define THREAD_LOCAL(type, var) __thread type var
#define RESTRICT __restrict__
#else
#define THREAD_LOCAL(type, var) type var
#define RESTRICT
#endif

#define LOCAL ATTRS(ATTR_NO_WARN_UNUSED_DEF)
#define LOCAL_CONSTANT ATTRS(ATTR_NO_WARN_UNUSED_DEF)

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

DEFS_BEGIN_NAMESPACE

#ifdef HU_COMP_MSVC
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned uint32_t;
typedef unsigned long long uint64_t;

typedef signed char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long long int64_t;
#else
#endif

DEFS_END_NAMESPACE

#endif
