#include "glt/Uniforms.hpp"

#include "err/err.hpp"
#include "glt/ShaderProgram.hpp"
#include "glt/color.hpp"
#include "glt/type_info.hpp"
#include "glt/utils.hpp"
#include "math/mat2.hpp"
#include "math/mat3.hpp"
#include "math/mat4.hpp"
#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"
#include "opengl.hpp"

namespace glt {

using namespace math;

namespace {

template<typename T>
auto
gltype(const T &x) -> gl_mapped_type_t<T>
{
    return x;
}

#if ENABLE_GLDEBUG_P
bl::string
descGLType(GLenum ty)
{

#    define CASE(ty)                                                           \
    case ty:                                                                   \
        return #ty;

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

    default:
        return "unknown OpenGL type";
    }

#    undef CASE
}
#endif

void
programUniform(GLuint program, GLint loc, float value)
{
    GL_CALL(glProgramUniform1fv, program, loc, 1, &value);
}

void
programUniform(GLuint program, GLint loc, bl::array_view<const float> value)
{
    GL_CALL(
      glProgramUniform1fv, program, loc, GLsizei(value.size()), value.data());
}

void
programUniform(GLuint program, GLint loc, const vec4_t &value)
{
    GL_CALL(glProgramUniform4fv, program, loc, 1, gltype(value).data());
}

void
programUniform(GLuint program, GLint loc, const vec3_t &value)
{
    GL_CALL(glProgramUniform3fv, program, loc, 1, gltype(value).data());
}

void
programUniform(GLuint program, GLint loc, const vec2_t &value)
{
    GL_CALL(glProgramUniform2fv, program, loc, 1, gltype(value).data());
}

void
programUniform(GLuint program, GLint loc, const mat4_t &value)
{
    GL_CALL(glProgramUniformMatrix4fv,
            program,
            loc,
            1,
            GL_FALSE,
            gltype(value).data());
}

void
programUniform(GLuint program, GLint loc, const mat3_t &value)
{
    GL_CALL(glProgramUniformMatrix3fv,
            program,
            loc,
            1,
            GL_FALSE,
            gltype(value).data());
}

void
programUniform(GLuint program, GLint loc, const mat2_t &value)
{
    GL_CALL(glProgramUniformMatrix2fv,
            program,
            loc,
            1,
            GL_FALSE,
            gltype(value).data());
}

void
programUniform(GLuint program, GLint loc, const BoundTexture &sampler)
{
    GL_CALL(glProgramUniform1i, program, loc, GLint(sampler.size_t));
}

void
programUniform(GLuint program, GLint loc, const Sampler &sampler)
{
    GL_CALL(glProgramUniform1i, program, loc, GLint(sampler.size_t));
}

void
programUniform(GLuint program, GLint loc, GLint val)
{
    GL_CALL(glProgramUniform1i, program, loc, val);
}

void
programUniform(GLuint program, GLint loc, GLuint val)
{
    GL_CALL(glProgramUniform1ui, program, loc, val);
}

template<typename T>
void
setUniform(bool mandatory,
           ShaderProgram &prog,
           const bl::string &name,
           GLenum type,
           const T &value)
{

    UNUSED(type);

    GLint locationi;
    GL_ASSIGN_CALL(
      locationi, glGetUniformLocation, *prog.program(), name.c_str());

    if (locationi == -1) {
        if (mandatory)
            ERR("unknown uniform: " + name);
        return;
    }

#if ENABLE_GLDEBUG_P

    GLint num_active;
    GL_CALL(glGetProgramiv, *prog.program(), GL_ACTIVE_UNIFORMS, &num_active);
    if (locationi < num_active) {
        auto location = GLuint(locationi);
        GLint actual_typei = -1;
        GL_CALL(glGetActiveUniformsiv,
                *prog.program(),
                1,
                &location,
                GL_UNIFORM_TYPE,
                &actual_typei);
        if (actual_typei != -1) {
            auto actual_type = GLenum(actual_typei);
            if (actual_type != type) {
                bl::string err =
                  "uniform \"" + name +
                  "\": types dont match, got: " + descGLType(type) +
                  ", expected: " + descGLType(actual_type);
                ERR(err.c_str());
                return;
            }
        }
    }

#endif

    programUniform(*prog.program(), locationi, value);
}

} // namespace

void
Uniforms::set(bool mandatory, const bl::string &name, float value)
{
    setUniform(mandatory, prog, name, GL_FLOAT, value);
}

void
Uniforms::set(bool mandatory, const bl::string &name, double value)
{
    setUniform(mandatory, prog, name, GL_FLOAT, float(value));
}

void
Uniforms::set(bool mandatory,
              const bl::string &name,
              bl::array_view<const float> value)
{
    setUniform(mandatory, prog, name, GL_FLOAT, value);
}

void
Uniforms::set(bool mandatory, const bl::string &name, const vec4_t &value)
{
    setUniform(mandatory, prog, name, GL_FLOAT_VEC4, value);
}

void
Uniforms::set(bool mandatory, const bl::string &name, const vec3_t &value)
{
    setUniform(mandatory, prog, name, GL_FLOAT_VEC3, value);
}

void
Uniforms::set(bool mandatory, const bl::string &name, const vec2_t &value)
{
    setUniform(mandatory, prog, name, GL_FLOAT_VEC2, value);
}

void
Uniforms::set(bool mandatory, const bl::string &name, const mat4_t &value)
{
    setUniform(mandatory, prog, name, GL_FLOAT_MAT4, value);
}

void
Uniforms::set(bool mandatory, const bl::string &name, const mat3_t &value)
{
    setUniform(mandatory, prog, name, GL_FLOAT_MAT3, value);
}

void
Uniforms::set(bool mandatory, const bl::string &name, const mat2_t &value)
{
    setUniform(mandatory, prog, name, GL_FLOAT_MAT2, value);
}

void
Uniforms::set(bool mandatory, const bl::string &name, color value)
{
    setUniform(mandatory, prog, name, GL_FLOAT_VEC4, value.vec4());
}

void
Uniforms::set(bool mandatory, const bl::string &name, GLint value)
{
    setUniform(mandatory, prog, name, GL_INT, value);
}

void
Uniforms::set(bool mandatory, const bl::string &name, GLuint value)
{
    setUniform(mandatory, prog, name, GL_UNSIGNED_INT, value);
}

void
Uniforms::set(bool mandatory,
              const bl::string &name,
              const BoundTexture &sampler)
{
    setUniform(mandatory, prog, name, sampler.type, sampler);
}

void
Uniforms::set(bool mandatory, const bl::string &name, const Sampler &sampler)
{
    setUniform(mandatory, prog, name, sampler.type, sampler);
}

GLenum
mapGLTextureType(GLenum texture_target)
{
    switch (texture_target) {
    case GL_TEXTURE_1D:
        return GL_SAMPLER_1D;
    case GL_TEXTURE_2D:
        return GL_SAMPLER_2D;
    case GL_TEXTURE_2D_MULTISAMPLE:
        return GL_SAMPLER_2D_MULTISAMPLE;
    case GL_TEXTURE_3D:
        return GL_SAMPLER_3D;
    default:
        ERR("invalid TextureTarget type");
        return GL_FLOAT_MAT4;
    }
}

} // namespace glt
