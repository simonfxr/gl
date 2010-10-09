#ifndef _SHADER_PROGRAM_H
#define _SHADER_PROGRAM_H

#include <string>
#include <GL/gl.h>

struct ShaderProgram {
    ShaderProgram();
    ~ShaderProgram();

    GLuint program;
    GLuint vertex_shader;
    GLuint fragment_shader;

    bool compileVertexShader(const std::string& source);
    bool compileVertexShaderFromFile(const std::string& path);

    bool compileFragmentShader(const std::string& source);
    bool compileFragmentShaderFromFile(const std::string& path);

    bool link();
};
#endif
