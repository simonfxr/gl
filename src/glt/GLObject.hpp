#ifndef GLT_GL_OBJECT_HPP
#define GLT_GL_OBJECT_HPP

#include "glt/conf.hpp"
#include "opengl.hpp"
#include "pp/enum.hpp"
#include "sys/io/Stream.hpp"
#include "util/noncopymove.hpp"

namespace glt {

// clang-format off
#define GLT_OBJECT_TYPE_ENUM_DEF(T, V0, V)                              \
    T(ObjectType, uint8_t,                                              \
      V0(Program)                                                       \
      V(Shader)                                                         \
      V(Buffer)                                                         \
      V(Framebuffer)                                                    \
      V(Renderbuffer)                                                   \
      V(Sampler)                                                        \
      V(Texture)                                                        \
      V(TransformFeedback)                                              \
      V(VertexArray)                                                    \
      V(Query))
// clang-format on

PP_DEF_ENUM_WITH_API(GLT_API, GLT_OBJECT_TYPE_ENUM_DEF);

template<ObjectType::enum_t>
struct GLObject;

using GLProgramObject = GLObject<ObjectType::Program>;
using GLShaderObject = GLObject<ObjectType::Shader>;
using GLBufferObject = GLObject<ObjectType::Buffer>;
using GLFramebufferObject = GLObject<ObjectType::Framebuffer>;
using GLRenderbufferObject = GLObject<ObjectType::Renderbuffer>;
using GLSamplerObject = GLObject<ObjectType::Sampler>;
using GLTextureObject = GLObject<ObjectType::Texture>;
using GLTransformFeedbackObject = GLObject<ObjectType::TransformFeedback>;
using GLVertexArrayObject = GLObject<ObjectType::VertexArray>;
using GLQueryObject = GLObject<ObjectType::Query>;

GLT_API void
generate(ObjectType t, GLsizei n, GLuint *names);

GLT_API void
release(ObjectType t, GLsizei n, const GLuint *names);

GLT_API void
generateShader(GLenum shader_type, GLuint *shader);

GLT_API void
printStats(sys::io::OutStream &);

template<ObjectType::enum_t T>
struct GLObjectBase : private NonCopyable
{
    static inline constexpr ObjectType type = T;
    GLuint _name;

    explicit constexpr GLObjectBase(GLuint name = 0) : _name(name) {}

    constexpr GLObjectBase(GLObjectBase &&rhs)
      : _name(std::exchange(rhs._name, 0))
    {}

    constexpr GLObjectBase &operator=(GLObjectBase &&rhs)
    {
        _name = std::exchange(rhs._name, 0);
        return *this;
    }

    ~GLObjectBase() { release(); }

    void release()
    {
        if (_name != 0)
            glt::release(type, 1, &_name);
        _name = 0;
    }

    bool valid() const { return _name != 0; }

    GLuint operator*() const { return _name; }
    GLuint &operator*() { return _name; }
};

template<ObjectType::enum_t T>
struct GLObject : public GLObjectBase<T>
{
    explicit GLObject(GLuint name = 0) : GLObjectBase<T>(name) {}
    void generate() { glt::generate(T, 1, &this->_name); }

    GLObject<T> &ensure()
    {
        if (this->_name == 0)
            generate();
        return *this;
    }
};

template<>
struct GLObject<ObjectType::Shader> : public GLObjectBase<ObjectType::Shader>
{
    explicit GLObject(GLuint name = 0) : GLObjectBase<ObjectType::Shader>(name)
    {}

    void generate(GLenum ty) { glt::generateShader(ty, &this->_name); }

    GLObject<ObjectType::Shader> &ensure(GLenum ty)
    {
        if (this->_name == 0)
            generate(ty);
        return *this;
    }
};

} // namespace glt

#endif
