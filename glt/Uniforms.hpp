#ifndef UNIFORMS_HPP
#define UNIFORMS_HPP

#include "defs.h"

#include <string>

namespace math {

struct vec3_t;
struct vec4_t;
struct mat4;
struct mat3;

}

using namespace math;

namespace glt {

struct ShaderProgram;
struct color;

struct Uniforms {
    ShaderProgram& prog;
    Uniforms(ShaderProgram& _prog) : prog(_prog) {}

    Uniforms& optional(const std::string& name, float value);
    Uniforms& optional(const std::string& name, const vec4_t& value);
    Uniforms& optional(const std::string& name, const vec3_t& value);
    Uniforms& optional(const std::string& name, const mat4& value);
    Uniforms& optional(const std::string& name, const mat3& value);
    Uniforms& optional(const std::string& name, color value);

    Uniforms& mandatory(const std::string& name, float value);
    Uniforms& mandatory(const std::string& name, const vec4_t& value);
    Uniforms& mandatory(const std::string& name, const vec3_t& value);
    Uniforms& mandatory(const std::string& name, const mat4& value);
    Uniforms& mandatory(const std::string& name, const mat3& value);
    Uniforms& mandatory(const std::string& name, color value);
};

} // namespace glt

#endif
