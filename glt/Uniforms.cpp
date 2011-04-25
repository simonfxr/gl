#include "Uniforms.hpp"

#include "opengl.h"
#include "math/vec3.hpp"
#include "math/vec4.hpp"
#include "math/mat4.hpp"
#include "math/mat3.hpp"
#include "color.hpp"

#include "glt/ShaderProgram.hpp"
#include "glt/utils.hpp"

namespace glt {

namespace {

struct Tex {
    TextureHandle& t;
    uint32 index;

    Tex(TextureHandle& _t, uint32 i) : t(_t), index(i) {}
};

template <typename T>
void set(GLint loc, const T& value);

std::string descGLType(GLenum ty) {

#define CASE(ty) case ty: return #ty;
    
    switch (ty) {
        CASE(GL_FLOAT);
        CASE(GL_FLOAT_VEC2);
        CASE(GL_FLOAT_VEC3);
        CASE(GL_FLOAT_VEC4);
        CASE(GL_INT);
        CASE(GL_INT_VEC2);
        CASE(GL_INT_VEC3);
        CASE(GL_INT_VEC4);
        CASE(GL_BOOL);
        CASE(GL_BOOL_VEC2);
        CASE(GL_BOOL_VEC3);
        CASE(GL_BOOL_VEC4);
        CASE(GL_FLOAT_MAT2);
        CASE(GL_FLOAT_MAT3);
        CASE(GL_FLOAT_MAT4);
        CASE(GL_FLOAT_MAT2x3);
        CASE(GL_FLOAT_MAT2x4);
        CASE(GL_FLOAT_MAT3x2);
        CASE(GL_FLOAT_MAT3x4);
        CASE(GL_FLOAT_MAT4x2);
        CASE(GL_FLOAT_MAT4x3);
        CASE(GL_SAMPLER_1D);
        CASE(GL_SAMPLER_2D);
        CASE(GL_SAMPLER_3D);
        CASE(GL_SAMPLER_CUBE);
        CASE(GL_SAMPLER_1D_SHADOW);
        CASE(GL_SAMPLER_2D_SHADOW);
    default: return "unknown OpenGL type";
    }

#undef CASE
}

GLenum mapGLTextureType(GLenum texture_target) {
    switch (texture_target) {
    case GL_TEXTURE_1D: return GL_SAMPLER_1D;
    case GL_TEXTURE_2D: return GL_SAMPLER_2D;
    case GL_TEXTURE_3D: return GL_SAMPLER_3D;
    default:
        ERR("invalid TextureTarget type");
        return GL_FLOAT_MAT4;
    }
}

template <typename T>
void setUniform(bool mandatory, const std::string& name, ShaderProgram& prog, GLenum type, const T& value) {

    UNUSED(type);
    
    GLint location;
    GL_CHECK(location = glGetUniformLocation(prog.program(), name.c_str()));
    
    if (location == -1) {
        if (mandatory)
            ERR("unknown uniform");
        return;
    }

#ifdef GLDEBUG

    GLenum actual_type;
    GL_CHECK(glGetActiveUniformsiv(prog.program(), 1, (const GLuint *) &location, GL_UNIFORM_TYPE, (GLint *) &actual_type));
    if (actual_type != type) {
        std::string err = "uniform types dont match, expected: " + descGLType(type) + ", actual type: " + descGLType(actual_type);
        ERR(err.c_str());
        return;
    }

#endif

    set(location, value);
}

template <>
void set(GLint loc, const float& value) {
    glUniform1fv(loc, 1, &value);
}

template <>
void set(GLint loc, const vec4_t& value) {
    glUniform4fv(loc, 1, &value.x);
}

template <>
void set(GLint loc, const vec3_t& value) {
    glUniform3fv(loc, 1, &value.x);
}

template <>
void set(GLint loc, const mat4_t& value) {
    glUniformMatrix4fv(loc, 1, GL_FALSE, value.components);
}

template <>
void set(GLint loc, const mat3_t& value) {
    glUniformMatrix3fv(loc, 1, GL_FALSE, value.components);
}

template <>
void set(GLint loc, const Tex& tex) {
    GL_CHECK(glUniform1i(loc, tex.index));
}

} // namespace anon

Uniforms& Uniforms::optional(const std::string& name, float value) {
    setUniform(false, name, prog, GL_FLOAT, value); return *this;
}

Uniforms& Uniforms::optional(const std::string& name, const vec4_t& value) {
    setUniform(false, name, prog, GL_FLOAT_VEC4, value); return *this;
}

Uniforms& Uniforms::optional(const std::string& name, const vec3_t& value) {
    setUniform(false, name, prog, GL_FLOAT_VEC3, value); return *this;
}

Uniforms& Uniforms::optional(const std::string& name, const mat4_t& value) {
    setUniform(false, name, prog, GL_FLOAT_MAT4, value); return *this;
}

Uniforms& Uniforms::optional(const std::string& name, const mat3_t& value) {
    setUniform(false, name, prog, GL_FLOAT_MAT3, value); return *this;
}

Uniforms& Uniforms::optional(const std::string& name, color value) {
    return optional(name, value.vec4()); 
}

Uniforms& Uniforms::optional(const std::string& name, TextureHandle& texture, uint32 active_tex) {
    setUniform(false, name, prog, mapGLTextureType(texture.glType()), Tex(texture, active_tex));
    return *this;
}

Uniforms& Uniforms::mandatory(const std::string& name, float value) {
    setUniform(true, name, prog, GL_FLOAT, value); return *this;
}

Uniforms& Uniforms::mandatory(const std::string& name, const vec4_t& value) {
    setUniform(true, name, prog, GL_FLOAT_VEC4, value); return *this;
}

Uniforms& Uniforms::mandatory(const std::string& name, const vec3_t& value) {
    setUniform(true, name, prog, GL_FLOAT_VEC3, value); return *this;
}

Uniforms& Uniforms::mandatory(const std::string& name, const mat4_t& value) {
    setUniform(true, name, prog, GL_FLOAT_MAT4, value); return *this;
}

Uniforms& Uniforms::mandatory(const std::string& name, const mat3_t& value) {
    setUniform(true, name, prog, GL_FLOAT_MAT3, value); return *this;
}

Uniforms& Uniforms::mandatory(const std::string& name, color value) {
    return mandatory(name, value.vec4());
}

Uniforms& Uniforms::mandatory(const std::string& name, TextureHandle& texture, uint32 active_tex) {
    setUniform(true, name, prog, mapGLTextureType(texture.glType()), Tex(texture, active_tex));
    return *this;
}

} // namespace glt
