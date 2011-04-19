#include <iostream>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <map>

#include <GL/glew.h>

#include "defs.h"
#include "glt/ShaderProgram.hpp"
#include "glt/ShaderManager.hpp"
#include "glt/utils.hpp"
#include "glt/Preprocessor.hpp"
#include "glt/GLSLPreprocessor.hpp"

#define LOG_LEVEL(data, lvl) ((data)->sm.verbosity() >= ShaderManager::lvl)

#define LOG(data, lvl, msg) do { if (LOG_LEVEL(data, lvl)) { (data)->sm.err() << msg; } } while (0)

#define LOG_INFO(data, msg) LOG(data, Info, msg)
#define LOG_ERROR(data, msg) LOG(data, OnlyErrors, msg)

namespace glt {

typedef std::map<std::string, ShaderProgram::ShaderType> ShaderMap;
typedef std::pair<std::string, ShaderProgram::ShaderType> ShaderMapping;

struct ShaderProgram::Data {
    Error last_error;
    GLuint program;
    ShaderManager& sm;
    ShaderMap shaders;

    Data(ShaderManager& _sm) :
        sm(_sm)
        {}

    void push_error(ShaderProgram::Error err) {
        if (last_error == NoError)
            last_error = err;
    }

    bool addShader(ShaderType type, const std::string& file, uint32 nsegs, const char *segments[], const GLint segLengths[]);
};

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
    self->shaders.clear();
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

bool ShaderProgram::Data::addShader(ShaderType type, const std::string& file, uint32 nsegments, const char *segments[], const GLint segLengths[]) {

    GLenum shader_type;
    if (!translateShaderType(type, &shader_type)) {
        LOG_ERROR(this, "unknown shader type" << std::endl);
        push_error(APIError);
        return false;
    }

    if (program == 0) {
        program = glCreateProgram();
        if (program == 0) {
            LOG_ERROR(this, "couldnt create program" << std::endl);
            push_error(OpenGLError);
            return false;
        }
    }

    GLuint shader = glCreateShader(shader_type);
    if (shader == 0) {
        LOG_ERROR(this, "couldnt create shader" << std::endl);
        push_error(OpenGLError);
        return false;
    }

    LOG_ERROR(this, "compiling " << file << " ... ");
        
    glShaderSource(shader, nsegments, segments, segLengths);
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
    return self->addShader(type, "<unknown>", 1, &str, &length);
}

bool ShaderProgram::addShaderFile(const std::string& file) {
    const char *begin = file.c_str();
    const char *end = begin + file.length();

    while (end > begin && end[-1] != '.')
        --end;

    ShaderType type;
    if (end > begin && strcmp(end, "vert") == 0) {
        type = VertexShader;
    } else if (end > begin && strcmp(end, "frag") == 0) {
        type = FragmentShader;
    } else {
        LOG_ERROR(self, "couldnt guess shader type based on file name" << std::endl);
        self->push_error(CompilationFailed);
        return false;
    }

    return addShaderFile(type, file);
}

bool ShaderProgram::addShaderFile(ShaderType type, const std::string& file) {

    DependencyHandler depHandler;
    bool success = false;
    std::string realname;

    {
        Preprocessor proc;
        IncludeHandler incHandler;
        ShaderContents shadersrc;

        depHandler.patches = &incHandler.includes;
    
        proc.installHandler("include", incHandler);
        proc.installHandler("need", depHandler);
        proc.err(self->sm.err());

        realname = self->sm.lookupPath(file);

        if (realname.empty()) {
            LOG_ERROR(self, "couldnt find file in path: " << file << std::endl);
            return false;
        } else if (self->shaders.count(realname) > 0) {
            return true;
        }

        self->shaders.insert(ShaderMapping(realname, type));

        if (!preprocess(self->sm, proc, realname, &incHandler.includes, shadersrc)) {
            LOG_ERROR(self, "couldnt process shader file" << std::endl);
            self->push_error(CompilationFailed);
            goto ret;
        }

        {
            uint32 nsegments = shadersrc.segments.size();
            const uint32 *segLengths = &shadersrc.segLengths[0];
            const char **segments = &shadersrc.segments[0];

            if (!self->addShader(type, realname, nsegments, segments, (GLint *) segLengths)) {
                goto ret;
            }
        }
    }

    for (uint32 i = 0; i < depHandler.deps.size(); ++i)
        if (!addShaderFile(depHandler.deps[i]))
            goto ret;

    success = true;

ret:

    if (!success) {
        self->shaders.erase(realname);
    }

    return success;
}

bool ShaderProgram::tryLink() {
    return !wasError() && link();
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
