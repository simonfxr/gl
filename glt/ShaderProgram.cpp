#include <iostream>
#include <cstdio>

#define GLEW_STATIC
#include <GL/glew.h>

#include "defs.h"
#include "glt/ShaderProgram.hpp"
#include "glt/ShaderManager.hpp"
#include "glt/utils.hpp"
#include "glt/include_proc.hpp"

#define LOG_LEVEL(data, lvl) ((data)->sm.verbosity() >= ShaderManager::lvl)

#define LOG(data, lvl, msg) do { if (LOG_LEVEL(data, lvl)) { (data)->sm.err() << msg; } } while (0)

#define LOG_INFO(data, msg) LOG(data, Info, msg)
#define LOG_ERROR(data, msg) LOG(data, OnlyErrors, msg)

namespace glt {

// namespace {

struct ShaderProgram::Data {
    Error last_error;
    GLuint program;
    ShaderManager& sm;

    Data(ShaderManager& _sm) :
        sm(_sm)
        {}

    void push_error(ShaderProgram::Error err) {
        if (last_error == NoError)
            last_error = err;
    }

    bool addShader(ShaderType type, const std::string& file, const FileContents& contents);
};

// } // namespace anon

ShaderProgram::ShaderProgram(ShaderManager& sm) :
    self(new Data(sm))
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

bool ShaderProgram::Data::addShader(ShaderProgram::ShaderType type, const std::string& file, const FileContents& src) {

    GLenum shader_type;
    if (!translateShaderType(type, &shader_type)) {
        LOG_ERROR(this, "unknown shader type");
        push_error(APIError);
        return false;
    }

    if (program == 0) {
        program = glCreateProgram();
        if (program == 0) {
            LOG_ERROR(this, "couldnt create program");
            push_error(OpenGLError);
            return false;
        }
    }

    GLuint shader = glCreateShader(shader_type);
    if (shader == 0) {
        LOG_ERROR(this, "couldnt create shader");
        push_error(OpenGLError);
        return false;
    }

    LOG_ERROR(this, "compiling " << file << " ... ");
        
    glShaderSource(shader, src.nsegments, src.segments, src.lengths);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    bool ok = success == GL_TRUE;
    LOG_ERROR(this, (ok ? "success" : "failed") << std::endl);

    if ((!ok && LOG_LEVEL(this, OnlyErrors)) || LOG_LEVEL(this, Info))
        printShaderLog(shader, sm.err());

    if (ok)
        glAttachShader(program, shader);
    else
        push_error(CompilationFailed);

    glDeleteShader(shader);

    return ok;
}

bool ShaderProgram::addShaderSrc(ShaderType type, const std::string& src) {
    const GLchar *str = src.c_str();
    GLsizei length = src.length();
    FileContents contents;
    contents.nsegments = 1;
    contents.segments = &str;
    contents.lengths = &length;
    return self->addShader(type, "<unknown>", contents);
}

bool ShaderProgram::addShaderFile(ShaderType type, const std::string& file) {

    FileContents contents;

    if (!readAndProcFile(file, self->sm.includeDirs(), contents)) {
        LOG_ERROR(self, "couldnt read shader file");
        self->push_error(CompilationFailed);
        return false;
    }
    
    bool ok = self->addShader(type, file, contents);

    deleteFileContents(contents);
    
    return ok;
}

bool ShaderProgram::link() {

    if (self->program == 0) {
        self->push_error(APIError);
        return false;
    }

    LOG_ERROR(self, "linking ... ");

    glLinkProgram(self->program);

    GLint success;
    glGetProgramiv(self->program, GL_LINK_STATUS, &success);
    bool ok = success == GL_TRUE;
    LOG_ERROR(self, (ok ? "success" : "failed") << std::endl);

    if (!ok)
        self->push_error(LinkageFailed);

    
    if ((!ok && LOG_LEVEL(self, OnlyErrors)) || LOG_LEVEL(self, Info))
        printProgramLog(self->program, self->sm.err());

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

    if (wasError() && LOG_LEVEL(self, Info)) {
        Error err = self->last_error;
        printError(self->sm.err());
        self->last_error = err;
    }
    
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
            printProgramLog(self->program, self->sm.err());
    }

    return valid == GL_TRUE;
}

} // namespace glt
