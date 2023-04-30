#ifndef FS_UNIX_HPP
#define FS_UNIX_HPP

#include "sys/conf.hpp"

#include <bl/string_view.hpp>

namespace sys::fs {

using namespace bl::literals;

const bl::cbyte SEPARATOR_CHAR = '/';
constexpr inline bl::string_view SEPARATOR = "/"_sv;

} // namespace sys::fs

#endif
