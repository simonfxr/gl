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


#define LOG_LEVEL(data, lvl) ((data)->sm.verbosity() >= ShaderManager::lvl)

#define LOG(data, lvl, msg) do { if (LOG_LEVEL(data, lvl)) { (data)->sm.err() << msg; } } while (0)

#define LOG_INFO(data, msg) LOG(data, Info, msg)
#define LOG_ERR(data, msg) LOG(data, OnlyErrors, msg)

namespace glt {

typedef ShaderManager::CachedShaderObject CachedShaderObject;

typedef std::map<std::string, Ref<CachedShaderObject> > ShaderMap;

typedef std::map<std::string, GLuint> Attributes;

struct ShaderProgram::Data {
    Error lastError;
    GLuint program;
    ShaderManager& sm;
    ShaderObjectCollection shaders;
    Attributes attrs;
    bool linked;

    Data(ShaderManager& _sm) :
        lastError(NoError),
        program(0),
        sm(_sm),
        shaders(),
        attrs(),
        linked(false)
        {}

    void pushError(ShaderProgram::Error err) {
        if (lastError == NoError)
            lastError = err;
    }

    bool createProgram();

    void printProgramLog(GLuint program);

    void handleCompileError();
};

ShaderProgram::ShaderProgram(ShaderManager& sm) :
    self(new Data(sm))
{}

ShaderProgram::ShaderProgram(const ShaderProgram& prog) {
    self(new Data(prog.self))
}

ShaderProgram::~ShaderProgram() {
    reset();
    delete self;
}

ShaderProgram& ShaderProgram::operator =(const ShaderProgram& prog) {
    if (prog != this)
        *self = *prog.self;

    return *this;
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

ShaderProgram::Error ShaderProgram::clearError() {
    Error err = self->lastError;
    self->lastError = NoError;
    return err;
}
    
bool ShaderProgram::wasError() {
    return self->lastError != NoError;
}

GLuint ShaderProgram::program() {
    ASSERT(self->createProgram());
    return self->program;
}

bool ShaderProgram::Data::createProgram() {
    if (program == 0) {
        GL_CHECK(program = glCreateProgram());
        if (program == 0) {
            LOG_ERR(this, "couldnt create program" << std::endl);
            pushError(OpenGLError);
            return false;
        }
    }
    return true;
}

void ShaderProgram::Data::printProgramLog(GLuint program) {
    std::ostream& out = sm.err();
    
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

bool ShaderProgram::addShaderSrc(ShaderType type, const std::string& src) {
    bool ok = self->sm.shaderCompiler().loadString(self->shaders, type, src);
    if !(ok) self->handleCompileError();
    else self->linked = false;
    return ok;
}

bool ShaderProgram::addShaderFile(const std::string& file, bool absolute) {
    return addShaderFile(file, ShaderProgram::GuessShaderType, absolute);
}

bool ShaderProgram::addShaderFile(ShaderType type, const std::string& file, bool absolute) {
    bool ok = self->sm.shaderCompiler()->loadFile(self->shaderes, type, file, absolute);
    if (!ok) self->handleCompileError();
    else self->linked = false;
    return ok;    
}

bool ShaderProgram::addShaderFilePair(const std::string& vert_file, const std::string& frag_file, bool absolute) {
    return addShaderFile(VertexShader, vert_file, absolute) &&
        addShaderFile(FragmentShader, frag_file, absolute);   
}

bool ShaderProgram::addShaderFilePair(const std::string& basename, bool absolute) {
    return addShaderFilePair(basename + ".vert", basename + ".frag", absolute);
}

bool ShaderProgram::tryLink() {
    return !wasError() && link();
}

bool ShaderProgram::link() {

    if (self->shaders.empty()) {
        LOG_ERR(self, LinkageFailed, "no shader objects to link");
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

    LOG_BEGIN(self, log::Error, "linking");

    GL_CHECK(glLinkProgram(self->program));

    GLint success;
    GL_CHECK(glGetProgramiv(self->program, GL_LINK_STATUS, &success));
    bool ok = success == GL_TRUE;
    LOG_PUT(self, ok ? "success" : "failed");
    LOG_END(self);

    if (!ok) {
        self->pushError(LinkageFailed);

        for (index_t i = 0; i < added.size(); ++i)
            GL_CHECK(glDetachShader(self->program, added[i]));
    }

    bool write_llog = true;
    log::Level lvl;

    if (!ok && LOG_LEVEL(self, log::Error))
        lvl = log::Error;
    else if (LOG_LEVEL(self, log::Info))
        lvl = log::Info;
    else
        write_llog = false;

    if (write_llog) {
        LOG_BEGIN(self, lvl);
        printProgramLog(self->program, LOG_DESTINATION(self));
        LOG_END(self);
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
        LOG_ERROR(self, log::Error, APIError, "program not linked");
        return;
    }

    if (wasError() && LOG_LEVEL(self, Info))
        LOG_ERROR(self, log::Info, err, "error occurred");
    
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
            new_prog.printError(self->sm.err());
            return false;
        }
    }

    return true;
}

GLint ShaderProgram::uniformLocation(const std::string& name) {
    GLint loc;
    GL_CHECK(loc = glGetUniformLocation(self->program, name.c_str()));
    if (loc == -1)
        self->pushError(UniformNotKnown);
    return loc;
}

void ShaderProgram::printError(std::ostream& out) {

    const char *err;
    switch (self->lastError) {
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
    bool ok = false;
    
    if (self->program == 0) {
        self->pushError(APIError);
        goto ret;
    }

    GL_CHECK(glValidateProgram(self->program));
    GLint valid;
    GL_CHECK(glGetProgramiv(self->program, GL_VALIDATE_STATUS, &valid));
    
    if (valid == GL_FALSE)
        self->pushError(ValidationFailed);
    else
        ok = true;

ret:

    if (!ok && printLogOnError)
        printProgramLog(self->program, self->sm.err());

    return ok;
}

Ref<CachedShaderObject> ShaderProgram::rebuildShaderObject(ShaderManager& sm, Ref<CachedShaderObject>& so) {

    ASSERT(so.ptr() != 0);

    sm.removeFromCache(*so);
    
    ShaderProgram prog(sm);
    if (!prog.addShaderFile(so->key, true)) {
        return ShaderManager::EMPTY_CACHE_ENTRY;
    }

    Ref<CachedShaderObject> new_so = prog.self->shaders[so->key];
    prog.self->shaders.clear();
    return new_so;
}

Ref<ShaderManager::CachedShaderObject> rebuildShaderObject(ShaderManager& self, Ref<ShaderManager::CachedShaderObject>& so) {
    return ShaderProgram::rebuildShaderObject(self, so);
}

bool ShaderProgram::bindAttributesGeneric(const VertexDescBase& desc) {
    for (uptr i = 0; i < desc.nattributes; ++i)
        if (!bindAttribute(desc.attributes[i].name, i))
            return false;
    return true;
}

} // namespace glt
