#ifndef GLT_GL_DEBUG_HPP
#define GLT_GL_DEBUG_HPP

#include "opengl.hpp"

#include "glt/conf.hpp"
#include "glt/utils.hpp"

#include "err/err.hpp"

#include <unordered_set>

namespace glt {

struct GLDebug
{
    std::unordered_set<GLuint> ignored;
    OpenGLVendor vendor;

    GLDebug();
    virtual ~GLDebug();
    void init();
    virtual void printDebugMessages(const err::Location &) = 0;
    bool shouldIgnore(GLuint);
    void ignoreMessage(OpenGLVendor vendor, GLuint id);

    GLDebug(const GLDebug &) = delete;
    GLDebug &operator=(const GLDebug &) = delete;
};

struct NoDebug : public GLDebug
{
    NoDebug() = default;
    ~NoDebug() override;
    virtual void printDebugMessages(const err::Location &) final override {}
};

struct ARBDebug : public GLDebug
{
    GLsizei message_buffer_length;
    char *message_buffer;

    explicit ARBDebug(GLsizei buf_len);
    ~ARBDebug() override;

    static GLDebug *init();
    virtual void printDebugMessages(const err::Location &loc) final override;
};

struct AMDDebug : public GLDebug
{
    GLsizei message_buffer_length;
    char *message_buffer;

    explicit AMDDebug(GLsizei buf_len);
    ~AMDDebug() override;

    static GLDebug *init();
    virtual void printDebugMessages(const err::Location &loc) final override;
};

} // namespace glt

#endif
