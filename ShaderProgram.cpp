#include <iostream>
#include <cstdio>

#include <GL/glew.h>
#include <GL/glut.h>

#include "defs.h"
#include "ShaderProgram.hpp"

using namespace std;

ShaderProgram::ShaderProgram() :
    program(0),
    vertex_shader(0),
    fragment_shader(0)
{}

ShaderProgram::~ShaderProgram() {
    reset();
}

void ShaderProgram::reset() {
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    glDeleteProgram(program);
    vertex_shader = 0;
    fragment_shader = 0;
    program = 0;
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

    cerr << "compiling " << file << " ... ";
        
    *shader = glCreateShader(type);
    glShaderSource(*shader, 1, &source, &source_len);
    glCompileShader(*shader);

    GLint success;
    glGetShaderiv(*shader, GL_COMPILE_STATUS, &success);
    bool ok = success == GL_TRUE;
    cerr << (ok ? "success" : "false") << endl;

    ShaderProgram::printShaderLog(*shader, cerr);

    if (!ok) {
        glDeleteShader(*shader);
        *shader = 0;
    }

    return ok;
}

} // namespace anon

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
    if (!readContents(path, src, len)) {
        std::cerr << "couldnt read file: " << path << std::endl;
        return false;
    }
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
    if (!readContents(path, src, len)) {
        std::cerr << "couldnt read file: " << path << std::endl;
        return false;
    }
    bool ok = createShader(GL_FRAGMENT_SHADER, path, src, len, &fragment_shader);
    delete[] src;
    return ok;
}

bool ShaderProgram::link() {

    if (program == 0 && (vertex_shader == 0 || fragment_shader == 0))
        return false;

    cerr << "linking ... ";

    if (program == 0) {
        program = glCreateProgram();
        glAttachShader(program, vertex_shader);
        glAttachShader(program, fragment_shader);
    }
    
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success == GL_FALSE) {
        cerr << "failed" << endl;
        glDeleteProgram(program);
    } else {
        cerr << "success" << endl;
    }

    printProgramLog(program, cerr);

    return success == GL_TRUE;
}

bool ShaderProgram::compileAndLink(const std::string& vertex_shader_file, const std::string& fragment_shader_file) {
    return
        compileVertexShaderFromFile(vertex_shader_file) &&
        compileFragmentShaderFromFile(fragment_shader_file) &&
        link();
}

bool ShaderProgram::bindAttribute(const std::string& s, GLuint position) {
    
    if (program == 0 && (vertex_shader == 0 || fragment_shader == 0))
        return false;
    
    if (program == 0) {
        program = glCreateProgram();
        glAttachShader(program, vertex_shader);
        glAttachShader(program, fragment_shader);
    }
    
    glBindAttribLocation(program, position, s.c_str());
    return true;
}

void ShaderProgram::printShaderLog(GLuint shader, std::ostream& out) {

    GLint log_len;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);
    if (log_len > 0) {
            
        out << "shader compile log:" << endl;
            
        GLchar *log = new GLchar[log_len];
            
        glGetShaderInfoLog(shader, log_len, NULL, log);
        
        out << log << endl
            << "end compile log" << endl;
        
        delete[] log;
    }
}

void ShaderProgram::printProgramLog(GLuint program, std::ostream& out) {

    GLint log_len;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_len);
    if (log_len > 0) {
        
        out << "link log:" << endl;
        GLchar *log = new GLchar[log_len];
        glGetProgramInfoLog(program, log_len, NULL, log);
    
        out << log << endl
            << "end link log" << endl;
    
        delete[] log;
    }
}

void ShaderProgram::use() {
    glUseProgram(program);
}
