#include <iostream>
#include <cstdio>

#define GLEW_STATIC
#include <GL/glew.h>

#include "defs.h"
#include "glt/ShaderProgram.hpp"
#include "glt/utils.hpp"
#include "glt/ShaderManager.hpp"

namespace glt {

namespace {

struct ShaderProgram::Data {
    Error last_error;
    GLuint program;

    void push_error(ShaderProgram::Error err) {
        if (last_error == NoError)
            last_error = err;
    }

    bool addShader(ShaderType type, const std::string& file, const GLchar *src, int32 src_len);
};

} // namespace anon

ShaderProgram::ShaderProgram(ShaderManager& sm) :
    self(new Data)
{
    self->last_error = NoError;
    self->program = 0;
}

ShaderProgram::~ShaderProgram() {
    reset();
    delete self;
}

void ShaderProgram::reset() {
    glDeleteProgram(self->program);
    self->program = 0;
    self->last_error = NoError;
}

ShaderProgram::Error ShaderProgram::clearError() {
    Error err = self->last_error;
    self->last_error = NoError;
    return err;
}
    
bool ShaderProgram::wasError() {
    return self->last_error != NoError;
}

GLuint ShaderProgram::program() {
    if (self->program == 0)
        self->program = glCreateProgram();
    return self->program;
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

bool readContents(const std::string& path, char * &file_contents, int32 &file_size) {
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

void printShaderLog(GLuint shader, std::ostream& out) {

    GLint log_len;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);
    if (log_len > 0) {
            
        out << "shader compile log:" << std::endl;
            
        GLchar *log = new GLchar[log_len];
            
        glGetShaderInfoLog(shader, log_len, NULL, log);
        
        out << log << std::endl
            << "end compile log" << std::endl;
        
        delete[] log;
    }
}

void printProgramLog(GLuint program, std::ostream& out) {

    GLint log_len;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_len);
    if (log_len > 0) {
        
        out << "link log:" << std::endl;
        GLchar *log = new GLchar[log_len];
        glGetProgramInfoLog(program, log_len, NULL, log);
    
        out << log << std::endl
            << "end link log" << std::endl;
    
        delete[] log;
    }
}

} // namespace anon

bool ShaderProgram::Data::addShader(ShaderProgram::ShaderType type, const std::string& file, const GLchar *src, int32 src_len) {

    GLenum shader_type;
    if (!translateShaderType(type, &shader_type)) {
        DEBUG_ERROR("unknown shader type");
        push_error(APIError);
        return false;
    }

    if (program == 0) {
        program = glCreateProgram();
        if (program == 0) {
            DEBUG_ERROR("couldnt create program");
            push_error(OpenGLError);
            return false;
        }
    }

    GLuint shader = glCreateShader(shader_type);
    if (shader == 0) {
        DEBUG_ERROR("couldnt create shader");
        push_error(OpenGLError);
        return false;
    }

    std::cerr << "compiling " << file << " ... ";
        
    glShaderSource(shader, 1, &src, &src_len);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    bool ok = success == GL_TRUE;
    std::cerr << (ok ? "success" : "failed") << std::endl;

#ifndef GLDEBUG
    if (!ok)
#endif
    {
        printShaderLog(shader, std::cerr);
    }

    if (ok)
        glAttachShader(program, shader);
    else
        push_error(CompilationFailed);

    glDeleteShader(shader);

    return ok;
}

bool ShaderProgram::addShaderSrc(ShaderType type, const std::string& src) {
    return self->addShader(type, "<unknown>", src.c_str(), src.length());
}

bool ShaderProgram::addShaderFile(ShaderType type, const std::string& file) {
    char *src;
    int32 len;
    
    if (!readContents(file, src, len)) {
        ERROR("couldnt read shader file");
        self->push_error(CompilationFailed);
        return false;
    }
    
    bool ok = self->addShader(type, file, src, len);
    delete[] src;
    return ok;
}

bool ShaderProgram::link() {

    if (self->program == 0) {
        self->push_error(APIError);
        return false;
    }

    std::cerr << "linking ... ";

    glLinkProgram(self->program);

    GLint success;
    glGetProgramiv(self->program, GL_LINK_STATUS, &success);
    bool ok = success == GL_TRUE;
    std::cerr << (ok ? "success" : "failed") << std::endl;
    
    if (!ok)
        self->push_error(LinkageFailed);

#ifndef GLDEBUG
    if (!ok)
#endif
    {
        printProgramLog(self->program, std::cerr);
    }

    return ok;
}

bool ShaderProgram::bindAttribute(const std::string& s, GLuint position) {
    if (self->program == 0)
        self->program = glCreateProgram();

    glBindAttribLocation(self->program, position, s.c_str());
    // FIXME: check wether attrib was added correctly
    
    return true;
}

void ShaderProgram::use() {

    if (self->program == 0) {
        self->push_error(APIError);
        return;
    }

#ifdef GLDEBUG

    if (wasError()) {
        Error err = self->last_error;
        printError(std::cerr);
        self->last_error = err;
    }
    
#endif
    
    GL_CHECK(glUseProgram(self->program));
}

bool ShaderProgram::replaceWith(ShaderProgram& new_prog) {
    
    if (new_prog.self->program != 0 && new_prog.self->last_error == NoError) {
        reset();
        self->program = new_prog.self->program;
        self->last_error = NoError;
        new_prog.self->program = 0;
        return true;
    }

    return false;
}

GLint ShaderProgram::uniformLocation(const std::string& name) {
    GLint loc;
    GL_CHECK(loc = glGetUniformLocation(self->program, name.c_str()));
    if (loc == -1)
        self->push_error(UniformNotKnown);
    return loc;
}

void ShaderProgram::printError(std::ostream& out) {

    const char *err;
    switch (self->last_error) {
    case NoError: err = 0; break;
    case CompilationFailed: err = "compilation failed"; break;
    case LinkageFailed: err = "linkage failed"; break;
    case AttributeNotBound: err = "couldnt bind attribute"; break;
    case UniformNotKnown: err = "uniform not known"; break;
    case ValidationFailed: err = "state validation failed"; break;
    case APIError: err = "error in api usage"; break;
    default: err = "unknown error"; break;
    }

    if (err != 0)
        out << "ShaderProgram error occurred: " << err << std::endl;
}

bool ShaderProgram::validate(bool printLogOnError) {
    
    if (self->program == 0) {
        self->push_error(APIError);
        return false;
    }

    GL_CHECK(glValidateProgram(self->program));
    GLint valid;
    GL_CHECK(glGetProgramiv(self->program, GL_VALIDATE_STATUS, &valid));
    
    if (valid == GL_FALSE) {
        self->push_error(ValidationFailed);

        if (printLogOnError)
            printProgramLog(self->program, std::cerr);
    }

    return valid == GL_TRUE;
}

} // namespace glt
