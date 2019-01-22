#ifndef BL_STRING_FWD_HPP
#define BL_STRING_FWD_HPP

#include "defs.h"

namespace bl {
template<typename CharT>
struct basic_string;

using string = basic_string<char>;

using wstring = basic_string<wchar_t>;
} // namespace bl

#endif
