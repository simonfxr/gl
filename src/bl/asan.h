#ifndef BL_ASAN_H
#define BL_ASAN_H

#include <hu/features.h>

#if hu_has_feature(address_sanitizer) || defined(__SANITIZE_ADDRESS__)
#    include <sanitizer/asan_interface.h>
#    define BL_ASAN_P 1
#    define BL_asan_poision_memory_region(addr, size)                          \
        (::std::is_constant_evaluated()                                        \
           ? (void) 0                                                          \
           : __asan_poison_memory_region((addr), (size)))
#    define BL_asan_unpoision_memory_region(addr, size)                        \
        (::std::is_constant_evaluated()                                        \
           ? (void) 0                                                          \
           : __asan_unpoison_memory_region((addr), (size)))
#else
#    define BL_ASAN_P 0
#    define BL_asan_poision_memory_region(addr, size)                          \
        ((void) (addr), (void) (size))
#    define BL_asan_unpoision_memory_region(addr, size)                        \
        ((void) (addr), (void) (size))
#endif

#endif // BL_ASAN_H
