#ifndef GLT_MODULE_HPP
#define GLT_MODULE_HPP

#include "glt/GLDebug.hpp"
#include "glt/utils.hpp"

#include <memory>

#ifdef DEFINE_GLT_MODULE
#    define GLT_MODULE_ACCESS
#else
#    define GLT_MODULE_ACCESS const
#endif

namespace glt {

struct Utils
{
    std::shared_ptr<GLDebug> gl_debug;

    bool print_opengl_calls = false;

    bool nv_mem_info_initialized = false;
    bool nv_mem_info_available = false;

    bool ati_mem_info_initialized = false;
    bool ati_mem_info_available = false;
    GLMemInfoATI::GLMemFree initial_free{};

    Utils();
};

struct Module
{
    Utils utils;
};

extern std::unique_ptr<Module> module;

} // namespace glt

#undef GLT_MODULE_ACCESS

#endif
