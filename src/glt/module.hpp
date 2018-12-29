#ifndef GLT_MODULE_HPP
#define GLT_MODULE_HPP

#include "data/Ref.hpp"
#include "glt/GLDebug.hpp"
#include "glt/utils.hpp"

#ifdef DEFINE_GLT_MODULE
#define GLT_MODULE_ACCESS
#else
#define GLT_MODULE_ACCESS const
#endif

namespace glt {

struct Utils
{
    Ref<GLDebug> gl_debug;

    bool print_opengl_calls = false;

    bool nv_mem_info_initialized = false;
    bool nv_mem_info_available = false;

    bool ati_mem_info_initialized = false;
    bool ati_mem_info_available = false;
    GLMemInfoATI::GLMemFree initial_free;

    Utils();
};

struct Module
{
    Utils utils;
};

extern Module *module;

} // namespace glt

#undef GLT_MODULE_ACCESS

#endif
