#ifndef UNIFORMS_HPP
#define UNIFORMS_HPP

#include "defs.h"

#include <string>

struct vec3;
struct vec4;
struct mat4;
struct mat3;
struct ShaderProgram;

namespace gltools {

struct color;

struct Uniforms {
    ShaderProgram& prog;
    Uniforms(ShaderProgram& _prog) : prog(_prog) {}

    Uniforms& optional(const std::string& name, float value);
    Uniforms& optional(const std::string& name, const vec4& value);
    Uniforms& optional(const std::string& name, const vec3& value);
    Uniforms& optional(const std::string& name, const mat4& value);
    Uniforms& optional(const std::string& name, const mat3& value);
    Uniforms& optional(const std::string& name, color value);

    Uniforms& mandatory(const std::string& name, float value);
    Uniforms& mandatory(const std::string& name, const vec4& value);
    Uniforms& mandatory(const std::string& name, const vec3& value);
    Uniforms& mandatory(const std::string& name, const mat4& value);
    Uniforms& mandatory(const std::string& name, const mat3& value);
    Uniforms& mandatory(const std::string& name, color value);
};

} // namespace gltools

#endif
