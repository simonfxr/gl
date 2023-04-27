#ifndef SYS_FS_WINDOWS_HPP
#define SYS_FS_WINDOWS_HPP

#include "sys/conf.hpp"

#include <bl/string_view.hpp>

namespace sys::fs {

using namespace bl::literals;

const bl::string_byte SEPARATOR_CHAR = '\\';
constexpr inline bl::string_view SEPARATOR = "\\"_sv;

} // namespace sys::fs

#endif
