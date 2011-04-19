#include <GL/glew.h>

#include "Uniforms.hpp"

#include "math/vec3.hpp"
#include "math/vec4.hpp"
#include "math/mat4.hpp"
#include "math/mat3.hpp"
#include "color.hpp"

#include "glt/ShaderProgram.hpp"
#include "glt/utils.hpp"

namespace glt {

namespace {

template <typename T>
void set(GLint loc, const T& value);

template <typename T>
void setUniform(bool mandatory, const std::string& name, ShaderProgram& prog, GLenum type, const T& value) {

    UNUSED(type);
    
    GLint location;
    GL_CHECK(location = glGetUniformLocation(prog.program(), name.c_str()));
    
    if (location == -1) {
        if (mandatory)
            ERROR("unknown uniform");
        return;
    }

#ifdef GLDEBUG

    GLenum actual_type;
    GL_CHECK(glGetActiveUniformsiv(prog.program(), 1, (const GLuint *) &location, GL_UNIFORM_TYPE, (GLint *) &actual_type));
    if (actual_type != type) {
        ERROR("uniform types dont match");
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

} // namespace glt
