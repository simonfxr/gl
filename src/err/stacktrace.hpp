#ifndef ERR_STACKTRACE_HPP
#define ERR_STACKTRACE_HPP

#include "bl/array_view.hpp"
#include "bl/string.hpp"
#include "bl/string_view.hpp"
#include "bl/vector.hpp"
#include "err/conf.hpp"
#include "sys/io/Stream_fwd.hpp"

#ifdef ENABLE_STACKTRACES
#    if HU_OS_LINUX_P || HU_OS_WINDOWS_P || __has_include(<execinfo.h>)
#        define HAVE_STACKTRACES_P 1
#    endif
#endif

#ifndef HAVE_STACKTRACES_P
#    define HAVE_STACKTRACES_P 0
#endif

namespace err {

struct FrameInfo
{
    bl::string function;
    bl::string demangled_function;
    bl::string file;
    int line{};
    void *ip{};
    bl::string module;
};

bl::array_view<FrameInfo>
stacktrace(bl::array_view<FrameInfo>,
           size_t skip = 0,
           bool demangle = true) noexcept;

bl::vector<FrameInfo>
stacktrace(size_t skip = 0,
           bool demangle = true,
           size_t max = size_t(-1)) noexcept;

void
print_stacktrace(sys::io::OutStream &out = sys::io::stderr(),
                 bl::string_view line_prefix = "",
                 size_t skip = 0,
                 bool demangle = true,
                 size_t max = size_t(-1)) noexcept;

} // namespace err

#endif
