#include <iostream>
#include <cstdio>

#include <GL/glew.h>
#include <GL/glut.h>

#include "defs.h"
#include "ShaderProgram.hpp"

using namespace std;

ShaderProgram::ShaderProgram() {
    vertex_shader = 0;
    fragment_shader = 0;
    program = 0;
}

ShaderProgram::~ShaderProgram() {

}

namespace {

    bool readContents(const string& path, char * &file_contents, int32 &file_size) {
        FILE *in = fopen(path.c_str(), "rb");
        if (in == 0)
            return false;
        if (fseek(in, 0, SEEK_END) == -1)
            return false;
        int64 size = ftell(in);
        if (size < 0)
            return false;
        if (fseek(in, 0, SEEK_SET) == -1)
            return false;
        char *contents = new char[size + 1];
        if (fread(contents, size, 1, in) != 1) {
            delete[] contents;
            return false;
        }
        contents[size] = '\0';
        file_contents = contents;
        file_size = size;
        return true;
    }

    bool createShader(GLenum type, const string& file, const GLchar *source, int32 source_len, GLenum *shader) {
        *shader = glCreateShader(type);
        glShaderSource(*shader, 1, &source, &source_len);
        glCompileShader(*shader);

        GLint success;

        glGetShaderiv(*shader, GL_COMPILE_STATUS, &success);
        if (success == GL_FALSE) {
            cerr << "failed to compile shader: " << file << endl;

            GLint log_len;
            glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &log_len);

            GLchar *log = new GLchar[log_len];
            glGetShaderInfoLog(*shader, log_len, NULL, log);

            cerr << log << endl;

            delete[] log;

            glDeleteShader(*shader);
            *shader = 0;

            return false;
        }

        return true;
    }

}

bool ShaderProgram::compileVertexShader(const string& source) {
    if (vertex_shader != 0)
        return false;
    return createShader(GL_VERTEX_SHADER, "<unknown>", source.c_str(), source.length(), &vertex_shader);
}

bool ShaderProgram::compileVertexShaderFromFile(const string& path) {
    if (vertex_shader != 0)
        return false;

    char *src;
    int32 len;
    if (!readContents(path, src, len))
        return false;
    bool ok = createShader(GL_VERTEX_SHADER, path, src, len, &vertex_shader);
    delete[] src;
    return ok;
}

bool ShaderProgram::compileFragmentShader(const string& source) {
    if (fragment_shader != 0)
        return false;
    return createShader(GL_FRAGMENT_SHADER, "<unknown>", source.c_str(), source.length(), &fragment_shader);
}

bool ShaderProgram::compileFragmentShaderFromFile(const string& path) {
    if (fragment_shader != 0)
        return false;

    char *src;
    int32 len;
    if (!readContents(path, src, len))
        return false;
    bool ok = createShader(GL_FRAGMENT_SHADER, path, src, len, &fragment_shader);
    delete[] src;
    return ok;
}

bool ShaderProgram::link() {
    if (program != 0 || vertex_shader == 0 || fragment_shader == 0)
        return false;

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success == GL_FALSE) {
        cerr << "failed to link shader program:" << endl;

        GLint log_len;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_len);

        GLchar *log = new GLchar[log_len];
        glGetProgramInfoLog(program, log_len, NULL, log);

        cerr << log << endl;

        delete[] log;

        glDeleteProgram(program);
        return false;
    }

    return true;
}

bool ShaderProgram::compileAndLink(const std::string& vertex_shader_file, const std::string& fragment_shader_file) {
    return
        compileVertexShaderFromFile(vertex_shader_file) &&
        compileFragmentShaderFromFile(fragment_shader_file) &&
        link();
}
