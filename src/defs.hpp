#ifndef DEFS_HPP_INCLUDED
#define DEFS_HPP_INCLUDED

#define DEFS_BEGIN_NAMESPACE namespace defs {
#define DEFS_END_NAMESPACE }

#include "defs.h"

#if HU_CXX_EXCEPTIONS_P
#error "compiling with exceptions not supported"
#endif

#if HU_CXX_RTTI_P
#error "compiling with rtti not supported"
#endif

namespace defs {

template<typename IntT>
inline size_t
_check_size_(IntT x)
{
    size_t s = size_t(x);
#ifdef SIGNED_SIZE
    /* DEBUG_ASSERT(s >= 0); */
#endif
    return s;
}

template<typename SizeT>
inline uintptr_t
_check_unsize_(SizeT s)
{
#ifdef SIGNED_SIZE

#endif
    return uintptr_t(s);
}

template<typename SizeT>
inline SizeT
_assert_size_(SizeT s)
{
#ifdef SIGNED_SIZE
//    ASSERT(s >= 0);
#endif
    return s;
}

#define SIZE(x) ::defs::_check_size_(x)
#define UNSIZE(x) ::defs::_check_unsize_(x)
#define ASSERT_SIZE(x) ::defs::_assert_size_(x)

#define DEF_ENUM_BITOR(ty)                                                     \
    inline ty operator|(ty a, ty b) { return ty(int(a) | int(b)); }

#define DECLARE_PIMPL(nm)                                                      \
    struct Data;                                                               \
    struct DataDeleter                                                         \
    {                                                                          \
        void operator()(Data *) noexcept;                                      \
    };                                                                         \
    const std::unique_ptr<Data, DataDeleter> nm

#define DECLARE_PIMPL_DEL(cl)                                                  \
    void cl::DataDeleter::operator()(Data *p) noexcept { delete p; }

} // namespace defs

#endif
