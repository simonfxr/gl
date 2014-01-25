#ifndef _DEFS_H_INCLUDED_
#define _DEFS_H_INCLUDED_

#define SIGNED_SIZE

#ifndef DEFS_BEGIN_NAMESPACE
#  define DEFS_BEGIN_NAMESPACE
#  define DEFS_END_NAMESPACE
#endif

#ifdef SYSTEM_WINDOWS
#  define WIN32_LEAND_AND_MEAN 1
#  define VC_EXTRALEAN 1
#  define NOMINMAX 1
#endif

#ifndef SYSTEM_WINDOWS
#  include <stdint.h>
#endif

#ifndef CXX11
#  undef CXX11_FINAL_OVERRIDE
#endif

#ifdef CXX11_FINAL_OVERRIDE
#  define OVERRIDE override
#  define FINAL final
#else
#  define OVERRIDE
#  define FINAL
#endif

#ifdef BUILD_SHARED
#  ifdef SYSTEM_WINDOWS
#    define SHARED_IMPORT __declspec(dllimport)
#    define SHARED_EXPORT __declspec(dllexport)
#  else
#    define SHARED_IMPORT __attribute__((visibility("default")))
#    define SHARED_EXPORT __attribute__((visibility("default")))
#  endif
#else
#  define SHARED_IMPORT
#  define SHARED_EXPORT
#endif

#ifdef GNU_EXTENSIONS
#  define likely(e) __builtin_expect(bool(e) != false, 1)
#  define unlikely(e) __builtin_expect(bool(e) != false, 0)
#else
#  define likely(e) (e)
#  define unlikely(e) (e)
#endif

#ifdef GNU_EXTENSIONS
#  define HAVE_ALIGNOF_EXPR
#  define HAVE_ALIGNOF_TYPE
#  define ALIGNOF_EXPR(e) __alignof__(e)
#  define ALIGNOF_TYPE(t) __alignof__(t)
#elif defined(COMPILER_CL)
#  define HAVE_ALIGNOF_TYPE
#  define ALIGNOF_TYPE(t) __alignof(t)
#  define ALIGNOF_EXPR(e) alignof_expr_not_defined
#else
#  define HAVE_ALIGNOF_TYPE
#  define ALIGNOF_TYPE(t) offsetof(struct { char ___c; t ___x; }, ___x)
#  define ALIGNOF_EXPR(e) alignof_expr_not_defined
#endif

#define ARRAY_LENGTH(x) ::defs::size(sizeof (x) / sizeof *(x))

#define UNUSED(x) ((void) (x))

#ifdef DEBUG
#  define ON_DEBUG(x) do { (x); } while (0)
#  define DEBUG_DECL(x) x
#else
#  define ON_DEBUG(x) ((void) 0)
#  define DEBUG_DECL(x)
#endif

#ifdef GNU_EXTENSIONS
#  define ATTRS(...) __attribute__((__VA_ARGS__))
#elif defined(COMPILER_CL)
#  define ATTRS(...) __declspec(__VA_ARGS)
#else
#  define ATTRS(...)
#endif

#ifdef GNU_EXTENSIONS
#  define ATTR_WARN_UNUSED warn_unused_result
#  define ATTR_NO_WARN_UNUSED_DEF unused
#  define ATTR_ALIGNED(n) aligned(n)
#  define ATTR_NOINLINE noinline
#  define ATTR_NOTHROW nothrow
#  define ATTR_FORCE_INLINE always_inline
#  define ATTR_NORETURN noreturn
#  define ATTR_PACKED packed
#elif defined(COMPILER_CL)
#  define ATTR_WARN_UNUSED
#  define ATTR_NO_WARN_UNUSED_DEF
#  define ATTR_ALIGNED(n) align(n)
#  define ATTR_NOINLINE noinline
#  define ATTR_NOTHROW nothrow
#  define ATTR_FORCE_INLINE
#  define ATTR_NORETURN noreturn
#  define ATTR_PACKED
#endif

#ifdef GNU_EXTENSIONS
#  define HAVE_THREAD_LOCAL
#  define THREAD_LOCAL(type, var) __thread type var
#  define RESTRICT __restrict__
#else
#  define THREAD_LOCAL(type, var) type var
#  define RESTRICT
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

#define STATIC_ASSERT(p) typedef char __compile_time_assert##__LINE__[1 - 2 * !(p)]

DEFS_BEGIN_NAMESPACE

#ifdef SYSTEM_WINDOWS

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned uint32;
typedef unsigned long long uint64;
    
typedef signed char int8;
typedef short int16;
typedef int int32;
typedef long long int64;

#  if PTR_BITS == 32
typedef int32 iptr;
typedef uint32 uptr;
#  else
typedef int64 iptr;
typedef uint64 uptr;
#  endif

#else
#  define IALIAS(t) typedef t##_t t

IALIAS(uint8);
IALIAS(uint16);
IALIAS(uint32);
IALIAS(uint64);

IALIAS(int8);
IALIAS(int16);
IALIAS(int32);
IALIAS(int64);

#  undef IALIAS

typedef uintptr_t uptr;
typedef intptr_t iptr;
#endif

typedef iptr int_t;
typedef uptr uint_t;

#if 0
/* some windows header defines small, use it as marker weather byte is typdef'ed */
#ifndef small
typedef uint8 byte;
#endif  
#endif // 0


#ifdef SIGNED_SIZE
typedef int32 size;
typedef int16 size16;
typedef int32 size32;
typedef int64 size64;
#else
typedef uint32 size;
typedef uint16 size16;
typedef uint32 size32;
typedef uint64 size64;
#endif

typedef size index;
typedef size16 index16;
typedef size32 index32;
typedef size64 index64;

DEFS_END_NAMESPACE

#endif
