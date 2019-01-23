#ifndef GLT_GL_DEBUG_HPP
#define GLT_GL_DEBUG_HPP

#include "bl/string.hpp"
#include "err/err.hpp"
#include "glt/conf.hpp"
#include "glt/utils.hpp"
#include "opengl_fwd.hpp"
#include "pp/pimpl.hpp"

namespace glt {

struct GLT_API GLDebug
{
    GLDebug();
    virtual ~GLDebug();
    void init();
    virtual void printDebugMessages(const err::Location &) = 0;
    bool shouldIgnore(GLuint);
    void ignoreMessage(OpenGLVendor vendor, GLuint id);

    GLDebug(const GLDebug &) = delete;
    GLDebug &operator=(const GLDebug &) = delete;

protected:
    DECLARE_PIMPL(GLT_API, self);
};

struct GLT_API NoDebug : public GLDebug
{
    NoDebug() = default;
    ~NoDebug() override;
    virtual void printDebugMessages(const err::Location &) final override {}
};

struct GLT_API ARBDebug : public GLDebug
{
    ARBDebug();
    ~ARBDebug() override;
    virtual void printDebugMessages(const err::Location &loc) final override;
    bool initialized() const { return _initialized; }

private:
    bl::string message_buffer;
    bool _initialized = false;
};

struct GLT_API AMDDebug : public GLDebug
{
    AMDDebug();
    ~AMDDebug() override;

    virtual void printDebugMessages(const err::Location &loc) final override;
    bool initialized() const { return _initialized; }

private:
    bl::string message_buffer;
    bool _initialized = false;
};

} // namespace glt

#endif
