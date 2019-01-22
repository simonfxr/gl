#ifndef BL_STRING_VIEW_FWD_HPP
#define BL_STRING_VIEW_FWD_HPP

#include "defs.h"

namespace bl {
template<typename CharT>
struct basic_string_view;

using string_view = basic_string_view<char>;

using wstring_view = basic_string_view<wchar_t>;
} // namespace bl

#endif
