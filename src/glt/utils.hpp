#ifndef GL_UTILS
#define GL_UTILS

#include "glt/conf.hpp"
#include "opengl.hpp"

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
isExtensionSupported(bl::string_view extension);

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
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return reinterpret_cast<const GLubyte *>(str);
}

bl::string_view
gl_unstr(const GLubyte *glstr) noexcept;

inline GLboolean
gl_bool(bool b)
{
    return static_cast<GLboolean>(b ? GL_TRUE : GL_FALSE);
}

inline bool
gl_unbool(GLboolean b)
{
    return b != static_cast<GLboolean>(GL_FALSE);
}

inline bool
gl_unbool(GLint b)
{
    return b != static_cast<GLint>(GL_FALSE);
}

#if defined(offsetof)
#    define GLT_OFFSETOF offsetof
#elif HU_COMP_GNULIKE_P || hu_has_builtin(__builtin_offsetof)
#    define GLT_OFFSETOF(t, fld) __builtin_offsetof(t, fld)
#else
#    error "NEITHER offsetof nor __builtin_offsetof available"
#endif

#define GLT_FIELD_OFFSET(type, field)                                          \
    ::glt::detail::gl_field_offset<type>(GLT_OFFSETOF(type, field))

namespace detail {

template<typename T>
constexpr GLvoid *
gl_field_offset(intptr_t offset)
{
    ASSERT(offset >= 0);
    ASSERT(static_cast<size_t>(offset) < sizeof(T));
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast,performance-no-int-to-ptr)
    return reinterpret_cast<GLvoid *>(offset);
}

} // namespace detail

} // namespace glt

#endif
