#include "glt/utils.hpp"

#include "err/err.hpp"
#include "glt/GLDebug.hpp"
#include "glt/module.hpp"
#include "opengl.hpp"

#include <cstring>

#define G (module->utils)

namespace glt {

bool
printOpenGLCalls()
{
    return G.print_opengl_calls;
}

void
printOpenGLCalls(bool yesno)
{
    G.print_opengl_calls = yesno;
}

bl::string
getGLErrorString(GLenum err)
{
    switch (err) {
#define err_case(e)                                                            \
    case e:                                                                    \
        return #e
        err_case(GL_NO_ERROR);
        err_case(GL_INVALID_ENUM);
        err_case(GL_INVALID_VALUE);
        err_case(GL_INVALID_OPERATION);
        err_case(GL_STACK_OVERFLOW);
        err_case(GL_STACK_UNDERFLOW);
        err_case(GL_OUT_OF_MEMORY);
        err_case(GL_TABLE_TOO_LARGE);
        err_case(GL_INVALID_FRAMEBUFFER_OPERATION);
#undef err_case
    default:
        sys::io::ByteStream rep;
        rep << "Unknown OpenGL error [code = " << err << "]";
        return rep.str();
    }
}

bool
printGLErrors(sys::io::OutStream &out)
{
    bool was_error = false;

    for (GLenum err; (err = glGetError()) != GL_NO_ERROR; was_error = true)
        out << "OpenGL error occurred: " << getGLErrorString(err)
            << sys::io::endl;

    return was_error;
}

bool
isExtensionSupported(const char *extension)
{
    GLint ni;
    glGetIntegerv(GL_NUM_EXTENSIONS, &ni);
    auto n = GLuint(ni);

    for (GLuint i = 0; i < n; i++)
        if (strcmp(extension, gl_unstr(glGetStringi(GL_EXTENSIONS, i))) == 0)
            return true;

    return false;
}

void
ignoreDebugMessage(OpenGLVendor vendor, GLuint id)
{
    G.gl_debug->ignoreMessage(vendor, id);
}

void
printGLError(const err::Location *loc, GLenum err)
{
    err::reportError(sys::io::stdout(),
                     "OpenGL Error",
                     *loc,
                     err::LogLevel::Error,
                     getGLErrorString(err).c_str());
}

void
printGLTrace(const err::Location *loc)
{
    if (G.print_opengl_calls)
        sys::io::stdout() << "OPENGL " << loc->operation << " " << loc->file
                          << ":" << loc->line << sys::io::endl;
}

bool
checkForGLError(const err::Location *loc)
{

    if (G.print_opengl_calls && loc->operation) {
        printGLTrace(loc);
    }

    bool was_error = false;

    for (GLenum err; (err = glGetError()) != GL_NO_ERROR; was_error = true)
        printGLError(loc, err);

    G.gl_debug->printDebugMessages(*loc);

    return was_error;
}

bool
initDebug()
{
    const char *debug_impl = nullptr;
    bool initialized = false;

    if (GLAD_GL_ARB_debug_output) {
        debug_impl = "GL_ARB_debug_output";
        sys::io::stdout() << "trying ARB debug" << sys::io::endl;
        auto dbg = bl::make_shared<ARBDebug>();
        if (dbg->initialized()) {
            G.gl_debug = dbg;
            initialized = true;
        }
    }

    if (!initialized && GLAD_GL_AMD_debug_output) {
        debug_impl = "GL_AMD_debug_output";
        sys::io::stdout() << "trying AMD debug" << sys::io::endl;
        auto dbg = bl::make_shared<AMDDebug>();
        if (dbg->initialized()) {
            G.gl_debug = dbg;
            initialized = true;
        }
    }

    if (!initialized) {
        sys::io::stdout()
          << "couldnt initialize Debug Output, no debug implementaion available"
          << sys::io::endl;
        G.gl_debug = bl::make_shared<NoDebug>();
    } else {
        sys::io::stdout() << "initialized Debug Output using " << debug_impl
                          << sys::io::endl;
    }

    return initialized;
}

namespace {

enum MemInfoField
{
    FREE_TOTAL,
    FREE_MAX_BLOCK,
    FREE_AUX_TOTAL,
    FREE_AUX_MAX_BLOCK,
    FREE_COUNT
};

void
queryMemFreeATI(GLMemInfoATI::GLMemFree *free)
{
    GLint info[FREE_COUNT];
    GL_CALL(glGetIntegerv, GL_VBO_FREE_MEMORY_ATI, info);
    free->freeVBO = info[FREE_TOTAL];
    GL_CALL(glGetIntegerv, GL_TEXTURE_FREE_MEMORY_ATI, info);
    free->freeTexture = info[FREE_TOTAL];
    GL_CALL(glGetIntegerv, GL_RENDERBUFFER_FREE_MEMORY_ATI, info);
    free->freeRenderbuffer = info[FREE_TOTAL];
}

} // namespace

bool
GLMemInfoATI::init()
{
    if (G.ati_mem_info_initialized)
        return G.ati_mem_info_available;
    G.ati_mem_info_initialized = true;
    G.ati_mem_info_available = bool(GLAD_GL_ATI_meminfo);
    if (G.ati_mem_info_available)
        queryMemFreeATI(&G.initial_free);
    return G.ati_mem_info_available;
}

bool
GLMemInfoATI::info(GLMemInfoATI *mi)
{

    if (!G.ati_mem_info_initialized) {
        ERR("GLMemInfoATI not initialized");
        return false;
    }

    if (!G.ati_mem_info_available)
        return false;

    queryMemFreeATI(&mi->current);
    mi->initial = G.initial_free;
    return true;
}

bool
GLMemInfoNV::init()
{
    if (G.nv_mem_info_initialized)
        return G.nv_mem_info_available;
    G.nv_mem_info_initialized = true;
    G.nv_mem_info_available = bool(GLAD_GL_NVX_gpu_memory_info);
    return G.nv_mem_info_available;
}

bool
GLMemInfoNV::info(GLMemInfoNV *mi)
{
    if (!G.nv_mem_info_initialized) {
        ERR("GLMemInfoNV not initialized");
        return false;
    }

    if (!G.nv_mem_info_available)
        return false;

    GLint total, total_dedicated, current, evicted, num_evictions;
    GL_CALL(
      glGetIntegerv, GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &total_dedicated);
    GL_CALL(
      glGetIntegerv, GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &total);
    GL_CALL(
      glGetIntegerv, GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &current);
    GL_CALL(
      glGetIntegerv, GL_GPU_MEMORY_INFO_EVICTION_COUNT_NVX, &num_evictions);
    GL_CALL(glGetIntegerv, GL_GPU_MEMORY_INFO_EVICTED_MEMORY_NVX, &evicted);

    mi->total = total;
    mi->total_dedicated = total_dedicated;
    mi->current = current;
    mi->evicted = evicted;
    mi->num_evictions = num_evictions;
    return true;
}

Utils::Utils() : gl_debug(bl::make_shared<NoDebug>()) {}

} // namespace glt
