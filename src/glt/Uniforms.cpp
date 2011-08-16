#include "opengl.hpp"

#ifdef min
#error "FOOO"
#endif


#include "math/vec3.hpp"
#include "math/vec4.hpp"
#include "math/mat3.hpp"
#include "math/mat4.hpp"

#include "glt/color.hpp"
#include "glt/ShaderProgram.hpp"
#include "glt/Uniforms.hpp"

#include "err/err.hpp"
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
        CASE(GL_DOUBLE);
        CASE(GL_DOUBLE_VEC2);
        CASE(GL_DOUBLE_VEC3);
        CASE(GL_DOUBLE_VEC4);
        CASE(GL_INT);
        CASE(GL_INT_VEC2);
        CASE(GL_INT_VEC3);
        CASE(GL_INT_VEC4);
        CASE(GL_UNSIGNED_INT);
        CASE(GL_UNSIGNED_INT_VEC2);
        CASE(GL_UNSIGNED_INT_VEC3);
        CASE(GL_UNSIGNED_INT_VEC4);
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
        CASE(GL_DOUBLE_MAT2);
        CASE(GL_DOUBLE_MAT3);
        CASE(GL_DOUBLE_MAT4);
        CASE(GL_DOUBLE_MAT2x3);
        CASE(GL_DOUBLE_MAT2x4);
        CASE(GL_DOUBLE_MAT3x2);
        CASE(GL_DOUBLE_MAT3x4);
        CASE(GL_DOUBLE_MAT4x2);
        CASE(GL_DOUBLE_MAT4x3);
        CASE(GL_SAMPLER_1D);
        CASE(GL_SAMPLER_2D);
        CASE(GL_SAMPLER_3D);
        CASE(GL_SAMPLER_CUBE);
        CASE(GL_SAMPLER_1D_SHADOW);
        CASE(GL_SAMPLER_2D_SHADOW);
        CASE(GL_SAMPLER_1D_ARRAY);
        CASE(GL_SAMPLER_2D_ARRAY);
        CASE(GL_SAMPLER_1D_ARRAY_SHADOW);
        CASE(GL_SAMPLER_2D_ARRAY_SHADOW);
        CASE(GL_SAMPLER_2D_MULTISAMPLE);
        CASE(GL_SAMPLER_2D_MULTISAMPLE_ARRAY);
        CASE(GL_SAMPLER_CUBE_SHADOW);
        CASE(GL_SAMPLER_BUFFER);
        CASE(GL_SAMPLER_2D_RECT);
        CASE(GL_SAMPLER_2D_RECT_SHADOW);
        CASE(GL_INT_SAMPLER_1D);
        CASE(GL_INT_SAMPLER_2D);
        CASE(GL_INT_SAMPLER_3D);
        CASE(GL_INT_SAMPLER_CUBE);
        CASE(GL_INT_SAMPLER_1D_ARRAY);
        CASE(GL_INT_SAMPLER_2D_ARRAY);
        CASE(GL_INT_SAMPLER_2D_MULTISAMPLE);
        CASE(GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY);
        CASE(GL_INT_SAMPLER_BUFFER);
        CASE(GL_INT_SAMPLER_2D_RECT);
        CASE(GL_UNSIGNED_INT_SAMPLER_1D);
        CASE(GL_UNSIGNED_INT_SAMPLER_2D);
        CASE(GL_UNSIGNED_INT_SAMPLER_3D);
        CASE(GL_UNSIGNED_INT_SAMPLER_CUBE);
        CASE(GL_UNSIGNED_INT_SAMPLER_1D_ARRAY);
        CASE(GL_UNSIGNED_INT_SAMPLER_2D_ARRAY);
        CASE(GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE);
        CASE(GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY);
        CASE(GL_UNSIGNED_INT_SAMPLER_BUFFER);
        CASE(GL_UNSIGNED_INT_SAMPLER_2D_RECT);
        
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
    
    GLint locationi;
    GL_CHECK(locationi = glGetUniformLocation(prog.program(), name.c_str()));
    
    if (locationi == -1) {
        if (mandatory)
            ERR("unknown uniform");
        return;
    }
    
    GLuint location = GLuint(locationi);

#ifdef GLDEBUG

    GLint actual_typei;
    GL_CHECK(glGetActiveUniformsiv(prog.program(), 1, &location, GL_UNIFORM_TYPE, &actual_typei));
    GLenum actual_type = GLenum(actual_typei);
    if (actual_type != type) {
        std::string err = "uniform \"" + name + "\": types dont match, got: " + descGLType(type) + ", expected: " + descGLType(actual_type);
        ERR(err.c_str());
        return;
    }

#endif

    set(locationi, value);
}

template <>
void set(GLint loc, const float& value) {
    glUniform1fv(loc, 1, &value);
}

template <>
void set(GLint loc, const vec4_t& value) {
    glUniform4fv(loc, 1, value.components);
}

template <>
void set(GLint loc, const vec3_t& value) {
    glUniform3fv(loc, 1, value.components);
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
    GL_CHECK(glUniform1i(loc, GLint(tex.index)));
}

template <>
void set(GLint loc, const GLint& val) {
    GL_CHECK(glUniform1i(loc, val));
}

template <>
void set(GLint loc, const GLuint& val) {
    GL_CHECK(glUniform1ui(loc, val));
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

Uniforms& Uniforms::optional(const std::string& name, GLint value) {
    setUniform(false, name, prog, GL_INT, value);
    return *this;
}

Uniforms& Uniforms::optional(const std::string& name, GLuint value) {
    setUniform(false, name, prog, GL_UNSIGNED_INT, value);
    return *this;
}

Uniforms& Uniforms::optional(const std::string& name, TextureHandle& texture, uint32 active_tex) {
    setUniform(false, name, prog, mapGLTextureType(texture.glType()), Tex(texture, active_tex));
    return *this;
}

Uniforms& Uniforms::optional(const std::string& name, TextureHandle& texture, uint32 active_tex, GLenum type) {
    setUniform(false, name, prog, type, Tex(texture, active_tex));
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

Uniforms& Uniforms::mandatory(const std::string& name, GLint value) {
    setUniform(true, name, prog, GL_INT, value);
    return *this;
}

Uniforms& Uniforms::mandatory(const std::string& name, GLuint value) {
    setUniform(true, name, prog, GL_UNSIGNED_INT, value);
    return *this;
}

Uniforms& Uniforms::mandatory(const std::string& name, TextureHandle& texture, uint32 active_tex) {
    setUniform(true, name, prog, mapGLTextureType(texture.glType()), Tex(texture, active_tex));
    return *this;
}

Uniforms& Uniforms::mandatory(const std::string& name, TextureHandle& texture, uint32 active_tex, GLenum type) {
    setUniform(true, name, prog, type, Tex(texture, active_tex));
    return *this;
}


} // namespace glt
