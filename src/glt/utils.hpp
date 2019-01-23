#ifndef GL_UTILS
#define GL_UTILS

#include "glt/conf.hpp"
#include "opengl_fwd.hpp"

#include "err/err.hpp"
#include "sys/io/Stream.hpp"

#if ENABLE_GLDEBUG_P
#    define GL_TRACE(msg)                                                      \
        do {                                                                   \
            ::glt::printGLTrace(ERROR_LOCATION_OP(msg));                       \
        } while (0)
#    define GL_CHECK(op)                                                       \
        do {                                                                   \
            (op);                                                              \
            ::glt::checkForGLError(ERROR_LOCATION_OP(PP_TOSTR(op)));           \
        } while (0)
#    define GL_CHECK_ERRORS() ::glt::checkForGLError(ERROR_LOCATION)
#else
#    define GL_CHECK(op) UNUSED(op)
#    define GL_CHECK_ERRORS() UNUSED(0)
#    define GL_TRACE(loc)
#endif

#define GL_CALL(fn, ...) GL_CHECK(fn(__VA_ARGS__))
#define GL_ASSIGN_CALL(var, fn, ...) GL_CHECK(var = fn(__VA_ARGS__))

#define GL_CALL_NO_CHECK(fn, ...)

namespace glt {

enum class OpenGLVendor : uint8_t
{
    Unknown,
    Nvidia,
    ATI,
    Intel
};

GLT_API bool
printOpenGLCalls();

GLT_API void
printOpenGLCalls(bool);

GLT_API void
printGLTrace(const err::Location *loc);

GLT_API bl::string
getGLErrorString(GLenum err);

GLT_API bool
printGLErrors(sys::io::OutStream &out);

GLT_API void
printGLError(const err::Location *loc, GLenum err);

GLT_API bool
checkForGLError(const err::Location *loc);

GLT_API bool
isExtensionSupported(const char *extension);

GLT_API bool
initDebug();

GLT_API void ignoreDebugMessage(OpenGLVendor, GLuint);

struct GLT_API GLMemInfoATI
{
    struct GLMemFree
    {
        // in kb
        size_t freeVBO;
        size_t freeTexture;
        size_t freeRenderbuffer;
    };

    GLMemFree current;
    GLMemFree initial;

    static bool init();
    static bool info(GLMemInfoATI *);
};

struct GLT_API GLMemInfoNV
{
    // in kb
    size_t total;
    size_t total_dedicated;
    size_t current;
    size_t evicted;
    size_t num_evictions;

    static bool init();
    static bool info(GLMemInfoNV *);
};

inline const GLubyte *
gl_str(const char *str)
{
    return reinterpret_cast<const GLubyte *>(str);
}

inline const char *
gl_unstr(const GLubyte *glstr)
{
    return reinterpret_cast<const char *>(glstr);
}

inline GLboolean
gl_bool(bool b)
{
    return GLboolean(b);
}

inline bool
gl_unbool(GLboolean b)
{
    return !!b;
}

} // namespace glt

#endif
