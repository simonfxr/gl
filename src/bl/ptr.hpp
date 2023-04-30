#ifndef BL_PTR_HPP
#define BL_PTR_HPP

#include <bl/non_null.hpp>
#include <bl/owned.hpp>
#include <bl/raw_ptr.hpp>

namespace bl {

template<typename T>
using ptr = non_null<raw_ptr<T>>;

template<typename T>
using nullable_ptr = raw_ptr<T>;

template<typename T, typename Deleter = default_deleter>
using owned_ptr = owned<ptr<T>, Deleter>;

template<typename T, typename Deleter = default_deleter>
using nullable_owned_ptr = owned<nullable_ptr<T>, Deleter>;

} // namespace bl

#endif // BL_PTR_HPP
