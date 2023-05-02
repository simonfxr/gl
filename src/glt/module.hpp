#ifndef GLT_MODULE_HPP
#define GLT_MODULE_HPP

#include "glt/GLDebug.hpp"
#include "glt/utils.hpp"

#include <bl/box.hpp>

namespace glt {

struct Utils
{
    GLDebug gl_debug;

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

extern bl::box<Module> mod;

} // namespace glt

#endif
