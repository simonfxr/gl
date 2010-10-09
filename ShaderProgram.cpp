
#include <iostream>
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

    string readContents(const string& path) {
        ifstream in(path);
        string contents = "";
        if (!in.is_open()) {
            cerr << "couldnt open " << path << endl;
            return "";
        } else {

            // noobig ohne ende... zero copy^^
            
            while (in.good()) { 
                string line;
                in >> line;
                contents += line;
            }
        }
    }

    bool createShader(GLenum type, const string& file, const string& source, GLenum *shader) {
        *shader = glCreateShader(type);
        const GLchar *src = source.c_str();
        const GLint len = source.length();
        glShaderSource(*shader, 1, &src, &len);
        glCompileShader(*shader);

        GLint succes;

        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
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

    return createShader(GL_VERTEX_SHADER, "<unknown>", source, &vertex_shader);
}

bool ShaderProgram::compileVertexShaderFromFile(const string& path) {
    if (vertex_shader != 0)
        return false;

    return createShader(GL_VERTEX_SHADER, path, getContents(path), &vertex_shader);
}

bool ShaderProgram::compileFragmentShader(const string& source) {
    if (fragment_shader != 0)
        return false;

    return createShader(GL_VERTEX_SHADER, "<unknown>", source, &fragment_shader);

}

bool ShaderProgram::compileFragmentShaderFromFile(const string& path) {
    if (fragment_shader != 0)
        return false;

    return createShader(GL_FRAGMENT_SHADER, path, getContents(path), &fragment_shader);
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
    if (!success) {
        cerr << "failed to link shader program:" << endl;

        GLint log_len;
        glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &log_len);

        GLchar *log = new GLchar[log_len];
        glGetShaderInfoLog(*shader, log_len, NULL, log);

        cerr << log << endl;

        delete[] log;

        glDeleteProgram(program);
        return false;
    }

    return true;
}
