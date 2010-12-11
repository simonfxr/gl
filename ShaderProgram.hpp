#ifndef _SHADER_PROGRAM_H
#define _SHADER_PROGRAM_H

#include <string>
#include <ostream>
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

    bool bindAttribute(const std::string& name, GLuint position);

    bool link();

    bool compileAndLink(const std::string& vertex_shader_file, const std::string& fragment_shader_file);

    static void printShaderLog(GLuint shader, std::ostream& out);
    static void printProgramLog(GLuint program, std::ostream& out);
};
#endif
