#ifndef BL_PTR_HPP
#define BL_PTR_HPP

#include <bl/non_null.hpp>

namespace bl {

template<typename T>
using ptr = non_null<T *>;

}

#endif // BL_PTR_HPP
