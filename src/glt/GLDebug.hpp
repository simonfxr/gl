#ifndef GLT_GL_DEBUG_HPP
#define GLT_GL_DEBUG_HPP

#include "err/err.hpp"
#include "glt/conf.hpp"
#include "glt/utils.hpp"
#include "opengl.hpp"
#include "util/NonCopyable.hpp"

#include <unordered_set>

namespace glt {

struct GLDebug : NonCopyable
{
    std::unordered_set<GLuint> ignored;
    OpenGLVendor vendor{};

    GLDebug();
    virtual ~GLDebug();
    void init();
    virtual void printDebugMessages(const err::Location &) = 0;
    bool shouldIgnore(GLuint);
    void ignoreMessage(OpenGLVendor vendor, GLuint id);
};

struct NoDebug : public GLDebug
{
    NoDebug() = default;
    ~NoDebug() override;
    void printDebugMessages(const err::Location & /*unused*/) final {}
};

struct ARBDebug : public GLDebug
{
    GLsizei message_buffer_length;
    char *message_buffer;

    explicit ARBDebug(GLsizei buf_len);
    ~ARBDebug() override;

    static GLDebug *init();
    void printDebugMessages(const err::Location &loc) final;
};

struct AMDDebug : public GLDebug
{
    GLsizei message_buffer_length;
    char *message_buffer;

    explicit AMDDebug(GLsizei buf_len);
    ~AMDDebug() override;

    static GLDebug *init();
    void printDebugMessages(const err::Location &loc) final;
};

} // namespace glt

#endif
