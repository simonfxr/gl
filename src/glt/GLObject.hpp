#ifndef GLT_GL_OBJECT_HPP
#define GLT_GL_OBJECT_HPP

#include "glt/conf.hpp"
#include "opengl.hpp"
#include "pp/enum.hpp"
#include "sys/io/Stream.hpp"

#include <bl/core.hpp>
#include <bl/noncopymove.hpp>

#include <compare>

namespace glt {

PP_DEF_ENUM(ObjectType,
            uint8_t,
            Program,
            Shader,
            Buffer,
            Framebuffer,
            Renderbuffer,
            Sampler,
            Texture,
            TransformFeedback,
            VertexArray,
            Query);

template<ObjectType>
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

template<ObjectType T>
struct GLObject : private bl::non_copyable
{
    static inline constexpr ObjectType type = T;
    GLuint _name{};

    constexpr GLObject() = default;
    explicit constexpr GLObject(GLuint name) : _name(name) {}

    constexpr GLObject(GLObject &&rhs) noexcept { swap(*this, rhs); }

    constexpr GLObject &operator=(GLObject &&rhs) noexcept
    {
        swap(*this, GLObject<T>{ bl::move(rhs) });
        return *this;
    }

    friend void swap(GLObject &lhs, GLObject &rhs) noexcept
    {
        std::swap(lhs._name, rhs._name);
    }

    ~GLObject() { release(); }

    void release()
    {
        if (auto n = bl::exchange(_name, 0); n)
            glt::release(type, 1, &n);
    }

    [[nodiscard]] bool valid() const { return _name != 0; }

    explicit operator bool() const noexcept { return valid(); }

    GLuint operator*() const { return _name; }
    GLuint &operator*() { return _name; }

    friend auto operator<=>(const GLObject &lhs, const GLObject &rhs) = default;

    void generate()
        requires(T != ObjectType::Shader)
    {
        glt::generate(T, 1, &this->_name);
    }

    void generate(GLenum ty)
        requires(T == ObjectType::Shader)
    {
        generateShader(ty, &_name);
    }

    template<typename... Args>
    GLObject<T> &ensure(Args &&...args)
    {
        if (!*this)
            generate(bl_fwd(args)...);
        return *this;
    }
};

} // namespace glt

#endif
