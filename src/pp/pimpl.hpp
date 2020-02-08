#ifndef UTILS_PIMPL_HPP
#define UTILS_PIMPL_HPP

#include <type_traits>

#define DECLARE_PIMPL_EXT(api, mod, nm)                                        \
    struct Data;                                                               \
    struct api DataDeleter                                                     \
    {                                                                          \
        void operator()(Data *) noexcept;                                      \
    };                                                                         \
    mod std::unique_ptr<Data, DataDeleter> nm

#define DECLARE_PIMPL(api, nm) DECLARE_PIMPL_EXT(api, const, nm)

#define DECLARE_MUT_PIMPL(api, nm) DECLARE_PIMPL_EXT(api, /* empty */, nm)

#define DECLARE_PIMPL_DEL_AUDIT(cl)                                            \
    void cl::DataDeleter::operator()(Data *p) noexcept                         \
    {                                                                          \
        static_assert(std::is_trivially_destructible_v<Data>);                 \
        delete p; /* NOLINT(cppcoreguidelines-owning-memory) */                \
    }

#define DECLARE_PIMPL_DEL(cl)                                                  \
    void cl::DataDeleter::operator()(Data *p) noexcept                         \
    {                                                                          \
        delete p; /* NOLINT(cppcoreguidelines-owning-memory) */                \
    }

#endif
