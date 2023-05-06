#include "glt/GLDebug.hpp"

#include "glt/conf.hpp"
#include "glt/utils.hpp"

#include <bl/optional.hpp>
#include <bl/vec.hpp>

#include <cstring>
#include <unordered_set>

namespace glt {

using namespace bl::literals;

template<typename T>
struct GLDebugBase : private bl::non_copyable
{
    std::unordered_set<GLuint> ignored;
    OpenGLVendor vendor{};

    void init();
    bool shouldIgnore(GLuint);

    friend void tag_invoke(ignoreGLDebugMessage_f,
                           GLDebugBase &d,
                           OpenGLVendor id_vendor,
                           GLuint id)
    {
        if (d.vendor != OpenGLVendor::Unknown && d.vendor == id_vendor)
            d.ignored.insert(id);
    }
};

struct NoDebug final : public GLDebugBase<NoDebug>
{
    NoDebug() = default;

    friend void tag_invoke(printGLDebugMessages_f,
                           NoDebug &,
                           const err::Location &)
    {}
};

struct ARBDebug final : public GLDebugBase<ARBDebug>
{
    bl::vec<char> message_buffer;

    explicit ARBDebug(GLsizei buf_len);

    static bl::optional<GLDebug> init();
    friend void tag_invoke(printGLDebugMessages_f,
                           ARBDebug &,
                           const err::Location &loc);
};

struct AMDDebug final : public GLDebugBase<AMDDebug>
{
    bl::vec<char> message_buffer;

    explicit AMDDebug(GLsizei buf_len);

    static bl::optional<GLDebug> init();

    friend void tag_invoke(printGLDebugMessages_f,
                           AMDDebug &,
                           const err::Location &loc);
};

template<typename T>
void
GLDebugBase<T>::init()
{
    const GLubyte *gl_vendor_str = glGetString(GL_VENDOR);
    if (gl_vendor_str == nullptr) {
        vendor = OpenGLVendor::Unknown;
        return;
    }

    const char *vendor_str =
      reinterpret_cast<const char *>(gl_vendor_str); // NOLINT
    if (strcmp(vendor_str, "NVIDIA Corporation") == 0)
        vendor = OpenGLVendor::Nvidia;
    else if (strcmp(vendor_str, "ATI Technologies") == 0)
        vendor = OpenGLVendor::ATI;
    else if (strcmp(vendor_str, "INTEL") == 0)
        vendor = OpenGLVendor::Intel;
    else
        vendor = OpenGLVendor::Unknown;
}

template<typename T>
bool
GLDebugBase<T>::shouldIgnore(GLuint id)
{
    return ignored.find(id) != ignored.end();
}

ARBDebug::ARBDebug(GLsizei buf_len) : message_buffer(buf_len)
{
    GLDebugBase::init();
}

bl::optional<GLDebug>
ARBDebug::init()
{
    glDebugMessageControlARB(
      GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        printGLError(ERROR_LOCATION, err);
        return bl::nullopt;
    }

    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);

    GLsizei max_len;
    glGetIntegerv(GL_MAX_DEBUG_MESSAGE_LENGTH_ARB, &max_len);

    return { GLDebug{ bl::in_place_type_t<ARBDebug>{}, max_len } };
}

void
tag_invoke(printGLDebugMessages_f, ARBDebug &d, const err::Location &loc)
{

    GLenum source;

    GLenum type;

    GLenum id;

    GLenum severity;
    GLsizei length;

    while (glGetDebugMessageLogARB(1,
                                   d.message_buffer.size(),
                                   &source,
                                   &type,
                                   &id,
                                   &severity,
                                   &length,
                                   d.message_buffer.data()) > 0) {

        if (d.shouldIgnore(id))
            continue;

#define sym_case(v, c)                                                         \
    case c:                                                                    \
        (v) = #c;                                                              \
        break

        const char *ssrc;
        switch (source) {
            sym_case(ssrc, GL_DEBUG_SOURCE_API_ARB);
            sym_case(ssrc, GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB);
            sym_case(ssrc, GL_DEBUG_SOURCE_SHADER_COMPILER_ARB);
            sym_case(ssrc, GL_DEBUG_SOURCE_THIRD_PARTY_ARB);
            sym_case(ssrc, GL_DEBUG_SOURCE_APPLICATION_ARB);
            sym_case(ssrc, GL_DEBUG_SOURCE_OTHER_ARB);
        default:
            ssrc = "unknown";
        }

        const char *stype;
        switch (type) {
            sym_case(stype, GL_DEBUG_TYPE_ERROR_ARB);
            sym_case(stype, GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB);
            sym_case(stype, GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB);
            sym_case(stype, GL_DEBUG_TYPE_PORTABILITY_ARB);
            sym_case(stype, GL_DEBUG_TYPE_PERFORMANCE_ARB);
            sym_case(stype, GL_DEBUG_TYPE_OTHER_ARB);
        default:
            stype = "unknown";
        }

        char other_severity_buf[32];
        const char *ssev;
        switch (severity) {
            sym_case(ssev, GL_DEBUG_SEVERITY_HIGH_ARB);
            sym_case(ssev, GL_DEBUG_SEVERITY_MEDIUM_ARB);
            sym_case(ssev, GL_DEBUG_SEVERITY_LOW_ARB);
        default:
            switch (severity) {
                sym_case(ssev, GL_DEBUG_SEVERITY_NOTIFICATION);
                sym_case(ssev, GL_DEBUG_SEVERITY_HIGH);
                sym_case(ssev, GL_DEBUG_SEVERITY_MEDIUM);
                sym_case(ssev, GL_DEBUG_SEVERITY_LOW);
            default:
                switch (severity) {
                    sym_case(ssev, GL_DEBUG_SEVERITY_HIGH);
                    sym_case(ssev, GL_DEBUG_SEVERITY_MEDIUM);
                    sym_case(ssev, GL_DEBUG_SEVERITY_LOW);
                    sym_case(ssev, GL_DEBUG_SEVERITY_NOTIFICATION);
                default:
                    snprintf(other_severity_buf,
                             sizeof other_severity_buf,
                             "0x%x",
                             severity);
                    ssev = other_severity_buf;
                }
            }
        }

#undef sym_case

        sys::io::ByteStream mesg;
        mesg << "source: "_k << bl::from_cstring(ssrc) << ", severity: "_k
             << bl::from_cstring(ssev) << ", type: "_k
             << bl::from_cstring(stype) << ", id: "_k << id << "  message: "_k
             << bl::from_cstring(d.message_buffer.data());

        err::reportError(sys::io::stdout(),
                         "OpenGL DEBUG",
                         loc,
                         err::LogLevel::Error,
                         bl::move(mesg).str());
    }
}

AMDDebug::AMDDebug(GLsizei buf_len) : message_buffer(buf_len)
{
    GLDebugBase::init();
}

bl::optional<GLDebug>
AMDDebug::init()
{

    glDebugMessageEnableAMD(0, 0, 0, nullptr, GL_TRUE);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        printGLError(ERROR_LOCATION, err);
        return bl::nullopt;
    }

    GLsizei max_len;
    glGetIntegerv(GL_MAX_DEBUG_MESSAGE_LENGTH_AMD, &max_len);

    return { GLDebug{ bl::in_place_type_t<AMDDebug>{}, max_len } };
}

void
tag_invoke(printGLDebugMessages_f, AMDDebug &d, const err::Location &loc)
{

    GLenum category;
    GLuint severity;

    GLuint id;
    GLsizei length;

    while (glGetDebugMessageLogAMD(1,
                                   d.message_buffer.size(),
                                   &category,
                                   &severity,
                                   &id,
                                   &length,
                                   d.message_buffer.data()) > 0) {

        if (d.shouldIgnore(id))
            continue;

#define sym_case(v, c)                                                         \
    case c:                                                                    \
        (v) = #c;                                                              \
        break

        const char *scat;
        switch (category) {
            sym_case(scat, GL_DEBUG_CATEGORY_API_ERROR_AMD);
            sym_case(scat, GL_DEBUG_CATEGORY_WINDOW_SYSTEM_AMD);
            sym_case(scat, GL_DEBUG_CATEGORY_DEPRECATION_AMD);
            sym_case(scat, GL_DEBUG_CATEGORY_UNDEFINED_BEHAVIOR_AMD);
            sym_case(scat, GL_DEBUG_CATEGORY_PERFORMANCE_AMD);
            sym_case(scat, GL_DEBUG_CATEGORY_SHADER_COMPILER_AMD);
            sym_case(scat, GL_DEBUG_CATEGORY_APPLICATION_AMD);
            sym_case(scat, GL_DEBUG_CATEGORY_OTHER_AMD);
        default:
            scat = "unknown";
        }

        const char *ssev;
        switch (severity) {
            sym_case(ssev, GL_DEBUG_SEVERITY_HIGH_AMD);
            sym_case(ssev, GL_DEBUG_SEVERITY_MEDIUM_AMD);
            sym_case(ssev, GL_DEBUG_SEVERITY_LOW_AMD);
        default:
            ssev = "unknown";
        }

#undef sym_case

        sys::io::ByteStream mesg;
        mesg << "category: "_k << bl::from_cstring(scat) << ", severity: "_k
             << bl::from_cstring(ssev) << ", id: "_k << id << "  message: "_k
             << bl::from_cstring(d.message_buffer.data());

        err::reportError(sys::io::stdout(),
                         "OpenGL DEBUG",
                         loc,
                         err::LogLevel::Error,
                         bl::move(mesg).str());
    }
}

GLDebug
makeGLDebugFallback()
{
    return GLDebug{ bl::in_place_type_t<NoDebug>{} };
}

GLDebug
makeGLDebug()
{

    bl::optional<GLDebug> dbg;
    bl::string_view debug_impl{};

    if (!dbg && GLAD_GL_ARB_debug_output) {
        debug_impl = "GL_ARB_debug_output"_k;
        sys::io::stdout() << "trying ARB debug\n"_k;
        dbg = ARBDebug::init();
    }

    if (!dbg && GLAD_GL_AMD_debug_output) {
        debug_impl = "GL_AMD_debug_output"_k;
        sys::io::stdout() << "trying AMD debug\n"_k;
        dbg = AMDDebug::init();
    }

    if (!dbg) {
        sys::io::stdout() << "couldnt initialize Debug Output, no debug "
                             "implementaion available"_k
                          << "\n"_k;
        return GLDebug{ bl::in_place_type_t<NoDebug>{} };
    } else {
        sys::io::stdout() << "initialized Debug Output using "_k << debug_impl
                          << "\n"_k;
        return bl::move(*dbg);
    }
}

} // namespace glt
