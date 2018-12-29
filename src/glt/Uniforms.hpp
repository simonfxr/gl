#ifndef UNIFORMS_HPP
#define UNIFORMS_HPP

#include "glt/conf.hpp"

#include "data/Array.hpp"
#include "glt/TextureSampler.hpp"
#include <string>

namespace math {

struct vec2_t;
struct vec3_t;
struct vec4_t;
struct mat4_t;
struct mat3_t;
struct mat2_t;

} // namespace math

using namespace math;

namespace glt {

struct ShaderProgram;
struct color;

GLT_API GLenum
mapGLTextureType(GLenum texture_target);

struct GLT_API BoundTexture
{
    GLenum type;
    uint32 index;

    BoundTexture() {}

    BoundTexture(GLenum ty, uint32 i) : type(ty), index(i) {}
};

struct GLT_API Sampler
{
    GLenum type;
    TextureSampler &sampler;
    uint32 index;

    Sampler(TextureSampler &_sampler, uint32 _index)
      : type(mapGLTextureType(_sampler.data()->glType()))
      , sampler(_sampler)
      , index(_index)
    {}
    Sampler(TextureSampler &_sampler, uint32 _index, GLenum _type)
      : type(_type), sampler(_sampler), index(_index)
    {}
};

struct GLT_API Uniforms
{
    ShaderProgram &prog;
    Uniforms(ShaderProgram &_prog) : prog(_prog) {}

private:
    void set(bool mandatory, const std::string &name, float value);
    void set(bool mandatory, const std::string &name, double value);
    void set(bool mandatory,
             const std::string &name,
             const Array<float> &value);
    void set(bool mandatory, const std::string &name, const vec4_t &value);
    void set(bool mandatory, const std::string &name, const vec3_t &value);
    void set(bool mandatory, const std::string &name, const vec2_t &value);
    void set(bool mandatory, const std::string &name, const mat4_t &value);
    void set(bool mandatory, const std::string &name, const mat3_t &value);
    void set(bool mandatory, const std::string &name, const mat2_t &value);
    void set(bool mandatory, const std::string &name, color value);
    void set(bool mandatory, const std::string &name, GLint value);
    void set(bool mandatory, const std::string &name, GLuint value);
    void set(bool mandatory,
             const std::string &name,
             const BoundTexture &sampler);
    void set(bool mandatory, const std::string &name, const Sampler &sampler);

public:
    template<typename T>
    Uniforms &optional(const std::string &name, const T &value)
    {
        set(false, name, value);
        return *this;
    }

    template<typename T>
    Uniforms &mandatory(const std::string &name, const T &value)
    {
        set(true, name, value);
        return *this;
    }
};

} // namespace glt

#endif
