#include <iostream>
#include <cstdio>
#include <cstring>
#include <map>
#include <set>
#include <sstream>

#include "defs.h"
#include "opengl.h"

#include "glt/ShaderProgram.hpp"
#include "glt/ShaderManager.hpp"
#include "glt/ShaderCompiler.hpp"
#include "glt/utils.hpp"

#include "error/error.hpp"

#include "sys/fs/fs.hpp"

#define RAISE_ERR(val, ec, msg) LOG_RAISE(val, ec, ::err::Error, msg)

template <>
struct LogTraits<glt::ShaderProgram> {
    static err::LogDestination getDestination(const glt::ShaderProgram& x) {
        return err::LogDestination(err::Info, const_cast<glt::ShaderProgram&>(x).shaderManager().err());
    }
};

namespace glt {

typedef std::map<std::string, GLuint> Attributes;

struct ShaderProgram::Data {
    ShaderProgram& self;
    Error lastError;
    GLuint program;
    ShaderManager& sm;
    ShaderObjectRoots shaders;
    Attributes attrs;
    bool linked;

    Data(ShaderProgram& owner, ShaderManager& _sm) :
        self(owner),
        lastError(NoError),
        program(0),
        sm(_sm),
        shaders(),
        attrs(),
        linked(false)
        {}

    Data(const Data& rhs) :
        self(rhs.self),
        lastError(rhs.lastError),
        program(0),
        sm(rhs.sm),
        shaders(rhs.shaders),
        attrs(rhs.attrs),
        linked(false)
        {}        

    bool createProgram();

    void printProgramLog(GLuint program, std::ostream& out);

    void handleCompileError();

    friend struct LogTraits<glt::ShaderProgram::Data>;
};

ShaderProgram::ShaderProgram(ShaderManager& sm) :
    self(new Data(*this, sm)) {}

ShaderProgram::ShaderProgram(const ShaderProgram& prog) :
    self(new Data(*prog.self)) {}

ShaderProgram::~ShaderProgram() {
    reset();
    delete self;
}

void ShaderProgram::reset() {
    GL_CHECK(glDeleteProgram(self->program));
    self->program = 0;
    self->lastError = NoError;
    self->shaders.clear();
    self->attrs.clear();
}

bool ShaderProgram::reload() {

    ShaderProgram new_prog(*this);
    bool outdated;

    if (!new_prog.self->sm.shaderCompiler().reloadRecursively(new_prog.self->shaders, &outdated))
        return false;

    if (outdated && new_prog.tryLink()) {
       return replaceWith(new_prog);
    } else {
        return false;
    }
}

void ShaderProgram::pushError(ShaderProgram::Error err) {
    if (self->lastError != NoError)
        self->lastError = err;
}

ShaderProgram::Error ShaderProgram::clearError() {
    Error err = self->lastError;
    self->lastError = NoError;
    return err;
}
    
bool ShaderProgram::wasError() {
    return self->lastError != NoError;
}

ShaderManager& ShaderProgram::shaderManager() {
    return self->sm;
}

GLuint ShaderProgram::program() {
    ASSERT(self->createProgram());
    return self->program;
}

void ShaderProgram::Data::handleCompileError() {
    self.pushError(ShaderProgram::CompilationFailed);
    sm.shaderCompiler().clearError();
}

bool ShaderProgram::Data::createProgram() {
    if (program == 0) {
        GL_CHECK(program = glCreateProgram());
        if (program == 0) {
            RAISE_ERR(self, ShaderProgram::OpenGLError, "couldnt create program");
            return false;
        }
    }
    return true;
}

void ShaderProgram::Data::printProgramLog(GLuint program, std::ostream& out) {
    GLint log_len;
    GL_CHECK(glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_len));
    
    if (log_len > 0) {
        
        GLchar *log = new GLchar[log_len];
        GLchar *logBegin = log;

        while (logBegin < log + log_len - 1 && isspace(*logBegin))
            ++logBegin;

        if (logBegin == log + log_len - 1) {
            out << "link log empty" << std::endl;
        } else {
            out << "link log: " << std::endl;
            GL_CHECK(glGetProgramInfoLog(program, log_len, NULL, log));
    
            out << log << std::endl
                << "end link log" << std::endl;
        }
            
        delete[] log;
    }
}

bool ShaderProgram::addShaderSrc(const std::string& src, ShaderManager::ShaderType type) {
    bool ok = self->sm.shaderCompiler().loadString(self->shaders, type, src);
    if (!ok) self->handleCompileError();
    else self->linked = false;
    return ok;
}

bool ShaderProgram::addShaderFile(const std::string& file, ShaderManager::ShaderType type, bool absolute) {
    bool ok = self->sm.shaderCompiler().loadFile(self->shaders, type, file, absolute);
    if (!ok) self->handleCompileError();
    else self->linked = false;
    return ok;    
}

bool ShaderProgram::addShaderFilePair(const std::string& vert_file, const std::string& frag_file, bool absolute) {
    return addShaderFile(vert_file, ShaderManager::VertexShader, absolute) &&
        addShaderFile(frag_file, ShaderManager::FragmentShader, absolute);   
}

bool ShaderProgram::addShaderFilePair(const std::string& basename, bool absolute) {
    return addShaderFilePair(basename + ".vert", basename + ".frag", absolute);
}

bool ShaderProgram::tryLink() {
    return !wasError() && link();
}

bool ShaderProgram::link() {

    if (self->shaders.empty()) {
        RAISE_ERR(*this, LinkageFailed, "no shader objects to link");
        return false;
    }
        
    if (!self->createProgram())
        return false;

    if (self->linked)
        return true;

    std::vector<GLuint> added;
    for (ShaderObjectRoots::const_iterator it = self->shaders.begin();
         it != self->shaders.end(); ++it) {
        GLuint shader = it->second->root->handle;
        GL_CHECK(glAttachShader(self->program, shader));
        added.push_back(shader);        
    }

    LOG_BEGIN(*this, err::Info);
    LOG_PUT(*this, "linking... ");

    GL_CHECK(glLinkProgram(self->program));

    GLint success;
    GL_CHECK(glGetProgramiv(self->program, GL_LINK_STATUS, &success));
    bool ok = success == GL_TRUE;
    LOG_PUT(*this, ok ? "success" : "failed");
    LOG_END(*this);

    if (!ok) {
        pushError(LinkageFailed);

        for (index_t i = 0; i < added.size(); ++i)
            GL_CHECK(glDetachShader(self->program, added[i]));
    }

    bool write_llog = true;
    err::LogLevel lvl;

    if (!ok && LOG_LEVEL(*this, err::Error))
        lvl = err::Error;
    else if (LOG_LEVEL(*this, err::Info))
        lvl = err::Info;
    else
        write_llog = false;

    if (write_llog) {
        LOG_BEGIN(*this, lvl);
        self->printProgramLog(self->program, LOG_DESTINATION(*this));
        LOG_END(*this);
    }

    if (ok)
        self->linked = true;

    return ok;
}

bool ShaderProgram::bindAttribute(const std::string& s, GLuint position) {
    if (!self->createProgram())
        return false;
    
    GL_CHECK(glBindAttribLocation(self->program, position, s.c_str()));

    self->attrs[s] = position;
    
    // FIXME: check wether attrib was added correctly
    
    return true;
}

void ShaderProgram::use() {

    if (self->program == 0 || !self->linked) {
        RAISE_ERR(*this, APIError, "program not linked");
        return;
    }

    if (wasError()) {
        LOG_BEGIN(*this, err::Info);
        LOG_PUT_ERR(*this, self->lastError, "using program despite error");
        LOG_END(*this);
    }
    
    GL_CHECK(glUseProgram(self->program));
}

bool ShaderProgram::replaceWith(ShaderProgram& new_prog) {
    if (&new_prog != this) {
        ASSERT(&new_prog.self->sm == &self->sm);

        if (new_prog.self->program != 0 && new_prog.self->lastError == NoError) {
            reset();
            self->program = new_prog.self->program;
            self->lastError = NoError;
            self->shaders = new_prog.self->shaders;
            new_prog.self->program = 0;
            new_prog.reset();
            return true;
        } else {
            LOG_BEGIN(*this, err::Info);
            LOG_PUT_ERR(*this, new_prog.self->lastError, "replaceWith failed");
            LOG_END(*this);
            return false;
        }
    }

    return true;
}

GLint ShaderProgram::uniformLocation(const std::string& name) {
    GLint loc;
    GL_CHECK(loc = glGetUniformLocation(self->program, name.c_str()));
    if (loc == -1)
        pushError(UniformNotKnown);
    return loc;
}

std::string ShaderProgram::stringError(Error e) {

    const char *err;
    switch (e) {
    case NoError: err = "no error"; break;
    case CompilationFailed: err = "compilation failed"; break;
    case LinkageFailed: err = "linkage failed"; break;
    case AttributeNotBound: err = "couldnt bind attribute"; break;
    case UniformNotKnown: err = "uniform not known"; break;
    case ValidationFailed: err = "state validation failed"; break;
    case APIError: err = "error in api usage"; break;
    default: err = "unknown error"; break;
    }

    return err;
}

bool ShaderProgram::validate(bool printLogOnError) {
    bool ok = false;
    
    if (self->program == 0) {
        pushError(APIError);
        goto ret;
    }

    GL_CHECK(glValidateProgram(self->program));
    GLint valid;
    GL_CHECK(glGetProgramiv(self->program, GL_VALIDATE_STATUS, &valid));
    
    if (valid == GL_FALSE)
        pushError(ValidationFailed);
    else
        ok = true;

ret:

    if (!ok && printLogOnError)
        self->printProgramLog(self->program, LOG_DESTINATION(*this));

    return ok;
}

bool ShaderProgram::bindAttributesGeneric(const VertexDescBase& desc) {
    for (uptr i = 0; i < desc.nattributes; ++i)
        if (!bindAttribute(desc.attributes[i].name, i))
            return false;
    return true;
}

} // namespace glt
