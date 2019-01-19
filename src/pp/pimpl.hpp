#ifndef UTILS_PIMPL_HPP
#define UTILS_PIMPL_HPP

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
