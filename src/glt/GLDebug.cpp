#include "glt/GLDebug.hpp"
#include "glt/conf.hpp"
#include "glt/utils.hpp"

#include <cstring>

namespace glt {

GLDebug::GLDebug() = default;

GLDebug::~GLDebug() = default;

void
GLDebug::init()
{
    const GLubyte *gl_vendor_str = glGetString(GL_VENDOR);
    if (gl_vendor_str == nullptr) {
        vendor = OpenGLVendor::Unknown;
        return;
    }

    const char *vendor_str = reinterpret_cast<const char *>(gl_vendor_str);
    if (strcmp(vendor_str, "NVIDIA Corporation") == 0)
        vendor = OpenGLVendor::Nvidia;
    else if (strcmp(vendor_str, "ATI Technologies") == 0)
        vendor = OpenGLVendor::ATI;
    else if (strcmp(vendor_str, "INTEL") == 0)
        vendor = OpenGLVendor::Intel;
    else
        vendor = OpenGLVendor::Unknown;
}

bool
GLDebug::shouldIgnore(GLuint id)
{
    return ignored.find(id) != ignored.end();
}

void
GLDebug::ignoreMessage(OpenGLVendor id_vendor, GLuint id)
{
    if (vendor != OpenGLVendor::Unknown && vendor == id_vendor)
        ignored.insert(id);
}

NoDebug::~NoDebug() = default;

ARBDebug::ARBDebug(GLsizei buf_len)
  : message_buffer_length(buf_len), message_buffer(new char[size_t(buf_len)])
{
    GLDebug::init();
}

ARBDebug::~ARBDebug()
{
    delete[] message_buffer;
}

GLDebug *
ARBDebug::init()
{
    glDebugMessageControlARB(
      GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        printGLError(ERROR_LOCATION, err);
        return nullptr;
    }

    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);

    GLsizei max_len;
    glGetIntegerv(GL_MAX_DEBUG_MESSAGE_LENGTH_ARB, &max_len);

    return new ARBDebug(max_len);
}

void
ARBDebug::printDebugMessages(const err::Location &loc)
{

    GLenum source, type, id, severity;
    GLsizei length;

    while (glGetDebugMessageLogARB(1,
                                   message_buffer_length,
                                   &source,
                                   &type,
                                   &id,
                                   &severity,
                                   &length,
                                   message_buffer) > 0) {

        if (shouldIgnore(id))
            continue;

#define sym_case(v, c)                                                         \
    case c:                                                                    \
        (v) = #c;                                                              \
        break

        const char *ssrc = "unknown";
        switch (source) {
            sym_case(ssrc, GL_DEBUG_SOURCE_API_ARB);
            sym_case(ssrc, GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB);
            sym_case(ssrc, GL_DEBUG_SOURCE_SHADER_COMPILER_ARB);
            sym_case(ssrc, GL_DEBUG_SOURCE_THIRD_PARTY_ARB);
            sym_case(ssrc, GL_DEBUG_SOURCE_APPLICATION_ARB);
            sym_case(ssrc, GL_DEBUG_SOURCE_OTHER_ARB);
        }

        const char *stype = "unknown";
        switch (type) {
            sym_case(stype, GL_DEBUG_TYPE_ERROR_ARB);
            sym_case(stype, GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB);
            sym_case(stype, GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB);
            sym_case(stype, GL_DEBUG_TYPE_PORTABILITY_ARB);
            sym_case(stype, GL_DEBUG_TYPE_PERFORMANCE_ARB);
            sym_case(stype, GL_DEBUG_TYPE_OTHER_ARB);
        }

        const char *ssev = "unknown";
        switch (severity) {
            sym_case(ssev, GL_DEBUG_SEVERITY_HIGH_ARB);
            sym_case(ssev, GL_DEBUG_SEVERITY_MEDIUM_ARB);
            sym_case(ssev, GL_DEBUG_SEVERITY_LOW_ARB);
        }

#undef sym_case

        sys::io::ByteStream mesg;
        mesg << "source: " << ssrc << ", severity: " << ssev
             << ", type: " << stype << ", id: " << id
             << "  message: " << message_buffer;

        err::reportError(sys::io::stdout(),
                         "OpenGL DEBUG",
                         loc,
                         err::LogLevel::Error,
                         mesg.str().c_str());
    }
}

AMDDebug::AMDDebug(GLsizei buf_len)
  : message_buffer_length(buf_len), message_buffer(new char[size_t(buf_len)])
{
    GLDebug::init();
}

AMDDebug::~AMDDebug()
{
    delete[] message_buffer;
}

GLDebug *
AMDDebug::init()
{

    glDebugMessageEnableAMD(0, 0, 0, nullptr, GL_TRUE);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        printGLError(ERROR_LOCATION, err);
        return nullptr;
    }

    GLsizei max_len;
    glGetIntegerv(GL_MAX_DEBUG_MESSAGE_LENGTH_AMD, &max_len);

    return new AMDDebug(max_len);
}

void
AMDDebug::printDebugMessages(const err::Location &loc)
{

    GLenum category;
    GLuint severity, id;
    GLsizei length;

    while (glGetDebugMessageLogAMD(1,
                                   message_buffer_length,
                                   &category,
                                   &severity,
                                   &id,
                                   &length,
                                   message_buffer) > 0) {

        if (shouldIgnore(id))
            continue;

#define sym_case(v, c)                                                         \
    case c:                                                                    \
        (v) = #c;                                                              \
        break

        const char *scat = "unknown";
        switch (category) {
            sym_case(scat, GL_DEBUG_CATEGORY_API_ERROR_AMD);
            sym_case(scat, GL_DEBUG_CATEGORY_WINDOW_SYSTEM_AMD);
            sym_case(scat, GL_DEBUG_CATEGORY_DEPRECATION_AMD);
            sym_case(scat, GL_DEBUG_CATEGORY_UNDEFINED_BEHAVIOR_AMD);
            sym_case(scat, GL_DEBUG_CATEGORY_PERFORMANCE_AMD);
            sym_case(scat, GL_DEBUG_CATEGORY_SHADER_COMPILER_AMD);
            sym_case(scat, GL_DEBUG_CATEGORY_APPLICATION_AMD);
            sym_case(scat, GL_DEBUG_CATEGORY_OTHER_AMD);
        }

        const char *ssev = "unknown";
        switch (severity) {
            sym_case(ssev, GL_DEBUG_SEVERITY_HIGH_AMD);
            sym_case(ssev, GL_DEBUG_SEVERITY_MEDIUM_AMD);
            sym_case(ssev, GL_DEBUG_SEVERITY_LOW_AMD);
        }

#undef sym_case

        sys::io::ByteStream mesg;
        mesg << "category: " << scat << ", severity: " << ssev << ", id: " << id
             << "  message: " << message_buffer;

        err::reportError(sys::io::stdout(),
                         "OpenGL DEBUG",
                         loc,
                         err::LogLevel::Error,
                         mesg.str().c_str());
    }
}

} // namespace glt
