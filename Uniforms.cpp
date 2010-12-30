#define GLEW_STATIC
#include <GL/glew.h>

#include "Uniforms.hpp"

#include "math/vec3.hpp"
#include "math/vec4.hpp"
#include "math/mat4.hpp"
#include "math/mat3.hpp"
#include "color.hpp"

#include "ShaderProgram.hpp"
#include "gltools.hpp"

namespace gltools {

namespace {

template <typename T>
void set(GLint loc, const T& value);

template <typename T>
void setUniform(bool mandatory, const std::string& name, ShaderProgram& prog, GLenum type, const T& value) {

    UNUSED(type);
    
    GLint location;
    GL_CHECK(location = glGetUniformLocation(prog.program, name.c_str()));
    
    if (!mandatory && location == -1)
        return;

#ifdef GLDEBUG

    GLenum actual_type;
    GL_CHECK(glGetActiveUniformsiv(prog.program, 1, (const GLuint *) &location, GL_UNIFORM_TYPE, (GLint *) &actual_type));
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
void set(GLint loc, const vec4& value) {
    glUniform4fv(loc, 1, &value.x);
}

template <>
void set(GLint loc, const vec3& value) {
    glUniform3fv(loc, 1, &value.x);
}

template <>
void set(GLint loc, const mat4& value) {
    glUniformMatrix4fv(loc, 1, GL_FALSE, value.flat);
}

template <>
void set(GLint loc, const mat3& value) {
    glUniformMatrix3fv(loc, 1, GL_FALSE, value.flat);
}

} // namespace anon

Uniforms& Uniforms::optional(const std::string& name, float value) {
    setUniform(false, name, prog, GL_FLOAT, value); return *this;
}

Uniforms& Uniforms::optional(const std::string& name, const vec4& value) {
    setUniform(false, name, prog, GL_FLOAT_VEC4, value); return *this;
}

Uniforms& Uniforms::optional(const std::string& name, const vec3& value) {
    setUniform(false, name, prog, GL_FLOAT_VEC3, value); return *this;
}

Uniforms& Uniforms::optional(const std::string& name, const mat4& value) {
    setUniform(false, name, prog, GL_FLOAT_MAT4, value); return *this;
}

Uniforms& Uniforms::optional(const std::string& name, const mat3& value) {
    setUniform(false, name, prog, GL_FLOAT_MAT3, value); return *this;
}

Uniforms& Uniforms::optional(const std::string& name, color value) {
    optional(name, static_cast<vec4>(value)); return *this;
}

Uniforms& Uniforms::mandatory(const std::string& name, float value) {
    setUniform(true, name, prog, GL_FLOAT, value); return *this;
}

Uniforms& Uniforms::mandatory(const std::string& name, const vec4& value) {
    setUniform(true, name, prog, GL_FLOAT_VEC4, value); return *this;
}

Uniforms& Uniforms::mandatory(const std::string& name, const vec3& value) {
    setUniform(true, name, prog, GL_FLOAT_VEC3, value); return *this;
}

Uniforms& Uniforms::mandatory(const std::string& name, const mat4& value) {
    setUniform(true, name, prog, GL_FLOAT_MAT4, value); return *this;
}

Uniforms& Uniforms::mandatory(const std::string& name, const mat3& value) {
    setUniform(true, name, prog, GL_FLOAT_MAT3, value); return *this;
}

Uniforms& Uniforms::mandatory(const std::string& name, color value) {
    mandatory(name, static_cast<vec4>(value)); return *this;
}

} // namespace gltools
