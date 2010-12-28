#include <iostream>
#include <cstdio>

#include <GL/glew.h>
#include <GL/glut.h>

#include "defs.h"
#include "ShaderProgram.hpp"
#include "gltools.hpp"

using namespace std;

ShaderProgram::ShaderProgram() :
    last_error(NoError),
    program(0)
{}

ShaderProgram::~ShaderProgram() {
    reset();
}

void ShaderProgram::reset() {
    glDeleteProgram(program);
    program = 0;
    last_error = NoError;
}

namespace {

bool translateShaderType(ShaderProgram::ShaderType type, GLenum *gltype) {
    switch (type) {
    case ShaderProgram::VertexShader:
        *gltype = GL_VERTEX_SHADER; return true;
    case ShaderProgram::FragmentShader:
        *gltype = GL_FRAGMENT_SHADER; return true;
    default: return false;
    }    
}

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

bool addShader(GLuint *program, ShaderProgram::ShaderType type, const string& file, const GLchar *src, int32 src_len) {

    GLenum shader_type;
    if (!translateShaderType(type, &shader_type)) {
        ERROR("unknown shader type");
        return false;
    }

    if (*program == 0) {
        *program = glCreateProgram();
        if (*program == 0) {
            ERROR("couldnt create program");
            return false;
        }
    }

    GLuint shader = glCreateShader(shader_type);
    if (shader == 0) {
        ERROR("couldnt create shader");
        return false;
    }

    cerr << "compiling " << file << " ... ";
        
    glShaderSource(shader, 1, &src, &src_len);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    bool ok = success == GL_TRUE;
    cerr << (ok ? "success" : "failed") << endl;

    ShaderProgram::printShaderLog(shader, cerr);

    if (ok)
        glAttachShader(*program, shader);

    glDeleteShader(shader);

    return ok;
}

} // namespace anon

bool ShaderProgram::addShaderSrc(ShaderType type, const std::string& src) {
    bool ok = addShader(&program, type, "<unknown>", src.c_str(), src.length());
    if (!ok)
        push_error(CompilationFailed);
    return ok;
}

bool ShaderProgram::addShaderFile(ShaderType type, const std::string& file) {
    char *src;
    int32 len;
    
    if (!readContents(file, src, len)) {
        ERROR("couldnt read shader file");
        return false;
    }
    
    bool ok = addShader(&program, type, file, src, len);
    delete[] src;
    if (!ok)
        push_error(CompilationFailed);
    return ok;
}

bool ShaderProgram::link() {

    if (program == 0) return false;

    cerr << "linking ... ";

    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success == GL_FALSE) {
        cerr << "failed" << endl;
        push_error(LinkageFailed);
    } else {
        cerr << "success" << endl;
    }

    printProgramLog(program, cerr);

    return success == GL_TRUE;
}

bool ShaderProgram::bindAttribute(const std::string& s, GLuint position) {
    if (program == 0)
        program = glCreateProgram();
    
    glBindAttribLocation(program, position, s.c_str());
    // FIXME: check wether attrib was added correctly
    
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
    GL_CHECK(glUseProgram(program));
}

bool ShaderProgram::replaceWith(ShaderProgram& new_prog) {
    
    if (new_prog.program != 0 && new_prog.last_error == NoError) {
        reset();
        program = new_prog.program;
        new_prog.program = 0;
        return true;
    }

    return false;
}

GLint ShaderProgram::uniformLocation(const std::string& name) {
    GLint loc;
    GL_CHECK(loc = glGetUniformLocation(program, name.c_str()));
    if (loc == -1)
        push_error(UniformNotKnown);
    return loc;
}

void ShaderProgram::printError(std::ostream& out) {

    const char *err = 0;
    switch (last_error) {
    case NoError: break;
    case CompilationFailed: err = "compilation failed"; break;
    case LinkageFailed: err = "linkage failed"; break;
    case AttributeNotBound: err = "couldnt bind attribute"; break;
    case UniformNotKnown: err = "uniform not known"; break;
    default: err = "unknown error"; break;
    }

    if (err != 0)
        out << "ShaderProgram error occurred: " << err << std::endl;
}
