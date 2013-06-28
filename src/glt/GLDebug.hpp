#ifndef GLT_GL_DEBUG_HPP
#define GLT_GL_DEBUG_HPP

#include "opengl.hpp"

#include "glt/conf.hpp"
#include "glt/GLDebug.hpp"

#include "err/err.hpp"

#include <set>

namespace glt {

struct GLDebug {
    std::set<GLint> ignored;
    OpenGLVendor vendor;

    GLDebug();
    virtual ~GLDebug();
    void init();
    virtual void printDebugMessages(const err::Location&) = 0;
    bool shouldIgnore(GLint);
    void ignoreMessage(OpenGLVendor vendor, GLuint id);

private:
    GLDebug(const GLDebug&);
    GLDebug& operator =(const GLDebug &);
};

struct NoDebug : public GLDebug {
    NoDebug() {}
    virtual void printDebugMessages(const err::Location&) FINAL OVERRIDE {}

private:
    NoDebug(const NoDebug&);
    NoDebug& operator =(const NoDebug&);
};

struct ARBDebug : public GLDebug {
    GLsizei message_buffer_length;
    char *message_buffer;

    explicit ARBDebug(GLsizei buf_len);
    ~ARBDebug();
    
    static GLDebug* init();
    virtual void printDebugMessages(const err::Location& loc) FINAL OVERRIDE;
    
private:
    ARBDebug(const ARBDebug&);
    ARBDebug& operator =(const ARBDebug&);
};

struct AMDDebug : public GLDebug {

    GLsizei message_buffer_length;
    char *message_buffer;

    explicit AMDDebug(GLsizei buf_len);
    ~AMDDebug();
    
    static GLDebug* init();
    virtual void printDebugMessages(const err::Location& loc) FINAL OVERRIDE;

private:
    AMDDebug(const AMDDebug&);
    AMDDebug& operator =(const AMDDebug&);
};

} // namespace glt

#endif
