#ifndef GLT_GL_OBJECT_HPP
#define GLT_GL_OBJECT_HPP

#include "glt/conf.hpp"
#include "opengl.hpp"
#include "pp/enum.hpp"
#include "sys/io/Stream.hpp"
#include "util/noncopymove.hpp"

#include <compare>

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
printStats(sys::io::OutStream &);

GLT_API void
generateShader(GLenum shader_type, GLuint *shader);

template<ObjectType::enum_t T>
struct GLObject : private NonCopyable
{
    static inline constexpr ObjectType type = T;
    GLuint _name{};

    constexpr GLObject() = default;
    explicit constexpr GLObject(GLuint name) : _name(name) {}

    constexpr GLObject(GLObject &&rhs) noexcept { swap(rhs); }

    constexpr GLObject &operator=(GLObject &&rhs) noexcept
    {
        GLObject<T>{ std::move(rhs) }.swap(*this);
        return *this;
    }

    void swap(GLObject &rhs) noexcept { std::swap(_name, rhs._name); }

    ~GLObject() { release(); }

    void release()
    {
        if (auto n = std::exchange(_name, 0); n)
            glt::release(type, 1, &n);
    }

    bool valid() const { return _name != 0; }

    explicit operator bool() const noexcept { return valid(); }

    GLuint operator*() const { return _name; }
    GLuint &operator*() { return _name; }

    friend auto operator<=>(const GLObject &lhs, const GLObject &rhs) = default;

    void generate() requires(T != ObjectType::Shader)
    {
        glt::generate(T, 1, &this->_name);
    }

    void generate(GLenum ty) requires(T == ObjectType::Shader)
    {
        generateShader(ty, &_name);
    }

    template<typename... Args>
    GLObject<T> &ensure(Args &&...args)
    {
        if (!*this)
            generate(std::forward<Args>(args)...);
        return *this;
    }
};

} // namespace glt

#endif
