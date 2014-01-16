#ifndef GLT_GL_OBJECT_HPP
#define GLT_GL_OBJECT_HPP

#include "opengl.hpp"
#include "glt/conf.hpp"
#include "sys/io/Stream.hpp"

namespace glt {

namespace ObjectType {

enum Type {
    Program,
    Shader,
    Buffer,
    Framebuffer,
    Renderbuffer,
    Sampler,
    Texture,
    TransformFeedback,
    VertexArray,
    Query,
    Count
};

} // namespace ObjectType

template <ObjectType::Type>
struct GLObject;

typedef GLObject<ObjectType::Program> GLProgramObject;
typedef GLObject<ObjectType::Shader> GLShaderObject;
typedef GLObject<ObjectType::Buffer> GLBufferObject;
typedef GLObject<ObjectType::Framebuffer> GLFramebufferObject;
typedef GLObject<ObjectType::Renderbuffer> GLRenderbufferObject;
typedef GLObject<ObjectType::Sampler> GLSamplerObject;
typedef GLObject<ObjectType::Texture> GLTextureObject;
typedef GLObject<ObjectType::TransformFeedback> GLTransformFeedbackObject;
typedef GLObject<ObjectType::VertexArray> GLVertexArrayObject;
typedef GLObject<ObjectType::Query> GLQueryObject;

GLT_API void generate(ObjectType::Type t, GLsizei n, GLuint *names);
GLT_API void release(ObjectType::Type t, GLsizei n, const GLuint *names);
GLT_API void generateShader(GLenum shader_type, GLuint *shader);
GLT_API void printStats(sys::io::OutStream&);

template <ObjectType::Type T>
struct GLObjectBase {
    static const ObjectType::Type type;
    GLuint _name;

    explicit GLObjectBase(GLuint name = 0) : _name(name) {}
    ~GLObjectBase() { release(); }

    void release() {
        if (_name != 0)
            glt::release(type, 1, &_name);
        _name = 0;
    }

    bool valid() const {
        return _name != 0;
    }

    GLuint operator *() const { return _name; }
    GLuint& operator *() { return _name; }

private:
	GLObjectBase(const GLObjectBase<T>&) = delete;
	GLObjectBase<T>& operator =(const GLObjectBase<T>&) = delete;
};

template <ObjectType::Type T>
const ObjectType::Type GLObjectBase<T>::type = T;

template <ObjectType::Type T>
struct GLObject : public GLObjectBase<T> {
    explicit GLObject(GLuint name = 0) :
        GLObjectBase<T>(name) {}
    void generate() {
        glt::generate(T, 1, &this->_name);
    }

    GLObject<T>& ensure() {
        if (this->_name == 0)
            generate();
        return *this;
    }

private:
	GLObject(const GLObject<T>&) = delete;
	GLObject<T>& operator =(const GLObject<T>&) = delete;
};

template <>
struct GLObject<ObjectType::Shader> :
        public GLObjectBase<ObjectType::Shader> {

    explicit GLObject(GLuint name = 0) : 
        GLObjectBase<ObjectType::Shader>(name) {}

    void generate(GLenum type) {
        glt::generateShader(type, &this->_name);
    }

    GLObject<ObjectType::Shader>& ensure(GLenum type) {
        if (this->_name == 0)
            generate(type);
        return *this;
    }
};

} // namespace glt

#endif
