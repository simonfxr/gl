#ifndef UTILS_PIMPL_HPP
#define UTILS_PIMPL_HPP

#include "bl/unique_ptr.hpp"

#define DECLARE_PIMPL_EXT(api, mod, nm)                                        \
    struct Data;                                                               \
    struct api DataDeleter                                                     \
    {                                                                          \
        void operator()(Data *) noexcept;                                      \
    };                                                                         \
    mod bl::unique_ptr<Data, DataDeleter> nm

#define DECLARE_PIMPL(api, nm) DECLARE_PIMPL_EXT(api, const, nm)

#define DECLARE_MUT_PIMPL(api, nm) DECLARE_PIMPL_EXT(api, /* empty */, nm)

#define DECLARE_PIMPL_DEL(cl)                                                  \
    void cl::DataDeleter::operator()(Data *p) noexcept { delete p; }

#endif
