#ifndef UNIFORMS_HPP
#define UNIFORMS_HPP

#include "defs.hpp"

#include <string>
#include "glt/TextureHandle.hpp"

namespace math {

struct vec3_t;
struct vec4_t;
struct mat4_t;
struct mat3_t;

}

using namespace math;

namespace glt {

struct ShaderProgram;
struct color;

GLenum mapGLTextureType(GLenum texture_target);

struct Sampler {
    GLenum type;
    TextureHandle& tex;
    uint32 index;

    Sampler(TextureHandle& _tex, uint32 _index) :
        type(mapGLTextureType(_tex.glType())),
        tex(_tex),
        index(_index)
        {}

    Sampler(TextureHandle& _tex, uint32 _index, GLenum _type) :
        type(_type),
        tex(_tex),
        index(_index)
        {}
};

struct Uniforms {
    ShaderProgram& prog;
    Uniforms(ShaderProgram& _prog) : prog(_prog) {}

private:
    void set(bool mandatory, const std::string& name, float value);
    void set(bool mandatory, const std::string& name, const vec4_t& value);
    void set(bool mandatory, const std::string& name, const vec3_t& value);
    void set(bool mandatory, const std::string& name, const mat4_t& value);
    void set(bool mandatory, const std::string& name, const mat3_t& value);
    void set(bool mandatory, const std::string& name, color value);
    void set(bool mandatory, const std::string& name, GLint value);
    void set(bool mandatory, const std::string& name, GLuint value);
    void set(bool mandatory, const std::string& name, const Sampler& sampler);

public:

    template <typename T>
    Uniforms& optional(const std::string& name, const T& value) {
        set(false, name, value);
        return *this;
    }

    template <typename T>
    Uniforms& mandatory(const std::string& name, const T& value) {
        set(true, name, value);
        return *this;
    }
};

} // namespace glt

#endif
