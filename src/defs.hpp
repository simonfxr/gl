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

#define DECLARE_PIMPL(api, nm)                                                 \
    struct Data;                                                               \
    struct api DataDeleter                                                     \
    {                                                                          \
        void operator()(Data *) noexcept;                                      \
    };                                                                         \
    const std::unique_ptr<Data, DataDeleter> nm

#define DECLARE_PIMPL_DEL(cl)                                                  \
    void cl::DataDeleter::operator()(Data *p) noexcept { delete p; }

#endif
