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

#include "sys/measure.hpp"
#include "sys/fs/fs.hpp"

#define RAISE_ERR(val, ec, msg) LOG_RAISE(val, ec, ::err::Error, msg)

template <>
struct LogTraits<glt::ShaderProgram> {
    static err::LogDestination getDestination(const glt::ShaderProgram& x) {
        return err::LogDestination(err::Info, const_cast<glt::ShaderProgram&>(x).shaderManager().err());
    }
};

namespace glt {

namespace ShaderProgramError {

std::string stringError(Type e) {

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

} // namespace ShaderProgramError

typedef std::map<std::string, GLuint> Attributes;

struct ShaderProgram::Data {
    ShaderProgram& self;
    GLuint program;
    ShaderManager& sm;
    ShaderObjects shaders;
    Attributes attrs;
    bool linked;

    Data(ShaderProgram& owner, ShaderManager& _sm) :
        self(owner),
        program(0),
        sm(_sm),
        shaders(),
        attrs(),
        linked(false)
        {}

    Data(const Data& rhs) :
        self(rhs.self),
        program(0),
        sm(rhs.sm),
        shaders(rhs.shaders),
        attrs(rhs.attrs),
        linked(false)
        {}        

    bool createProgram();

    void printProgramLog(GLuint program, std::ostream& out);

    void handleCompileError(ShaderCompilerError::Type);
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
    clearError();
    self->shaders.clear();
    self->attrs.clear();
}

bool ShaderProgram::reload() {
    ShaderObjects newshaders;
    CompileState cstate(self->sm.shaderCompiler(), newshaders);

    for (ShaderObjects::iterator it = self->shaders.begin();
         it != self->shaders.end(); ++it) {
        Ref<CompileJob> job = CompileJob::reload(it->second);
        cstate.enqueue(job);
    }

    cstate.compileAll();
    if (cstate.wasError())
        return false;

    bool unchanged = newshaders.size() == self->shaders.size();
    if (unchanged) {
        ShaderObjects::const_iterator it1 = self->shaders.begin(),
                                      it2 = newshaders.begin();
        for (; it1 != self->shaders.end() && it2 != newshaders.end(); ++it1, ++it2) {
            if (!it1->second.same(it2->second)) {
                unchanged = false;
                break;
            }
        }
    }
    
    if (unchanged)
        return true;

    ShaderProgram new_prog(*this);
    new_prog.self->shaders = newshaders;
    if (new_prog.tryLink()) {
        return replaceWith(new_prog);
    } else {
        return false;
    }
}

ShaderManager& ShaderProgram::shaderManager() {
    return self->sm;
}

GLuint ShaderProgram::program() {
    ASSERT(self->createProgram());
    return self->program;
}

bool ShaderProgram::Data::createProgram() {
    if (program == 0) {
        GL_CHECK(program = glCreateProgram());
        if (program == 0) {
            RAISE_ERR(self, ShaderProgramError::OpenGLError, "couldnt create program");
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

void ShaderProgram::Data::handleCompileError(ShaderCompilerError::Type) {
    ERR("compile error occurred"); // FIXME
}

bool ShaderProgram::addShaderSrc(const std::string& src, ShaderManager::ShaderType type) {
    CompileState cstate(self->sm.shaderCompiler(), self->shaders);
    {
        Ref<CompileJob> job = CompileJob::load(makeRef<ShaderSource>(new StringSource(type, src)));
        cstate.enqueue(job);
    }
    
    cstate.compileAll();
    
    if (cstate.wasError()) {
        self->handleCompileError(cstate.getError());
        return false;
    }

    return true;
}

bool ShaderProgram::addShaderFile(const std::string& file0, ShaderManager::ShaderType type, bool absolute) {
    std::string file = file0;
    
    if (!absolute) {
        file = sys::fs::lookup(self->sm.shaderDirectories(), file);
        if (file.empty()) {
            RAISE_ERR(*this, ShaderProgramError::FileNotInPath,
                      "couldnt find file in shader directories: " + file);
            return false;
        }
    }

    file = sys::fs::absolutePath(file);
    ASSERT(!file.empty());
    
    CompileState cstate(self->sm.shaderCompiler(), self->shaders);
    {
        Ref<CompileJob> job = CompileJob::load(makeRef<ShaderSource>(new FileSource(type, file)));
        cstate.enqueue(job);
    }

    cstate.compileAll();
    
    if (cstate.wasError()) {
        self->handleCompileError(cstate.getError());
        return false;
    }

    return true;
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
        RAISE_ERR(*this, ShaderProgramError::LinkageFailed, "no shader objects to link");
        return false;
    }
        
    if (!self->createProgram())
        return false;

    if (self->linked)
        return true;

    std::vector<GLuint> added;
    for (ShaderObjects::const_iterator it = self->shaders.begin();
         it != self->shaders.end(); ++it) {
        GLuint shader = it->second->handle;
        GL_CHECK(glAttachShader(self->program, shader));
        added.push_back(shader);        
    }

    LOG_BEGIN(*this, err::Info);
    LOG_PUT(*this, "linking ... ");

    float wct;
    measure_time(wct, glLinkProgram(self->program));
    GL_CHECK_ERRORS();

    GLint success;
    GL_CHECK(glGetProgramiv(self->program, GL_LINK_STATUS, &success));
    bool ok = success == GL_TRUE;
    LOG_PUT(*this, ok ? "success" : "failed") << " (" << (wct * 1000) << " ms)" << std::endl;
    LOG_END(*this);

    if (!ok) {
        pushError(ShaderProgramError::LinkageFailed);

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
        RAISE_ERR(*this, ShaderProgramError::APIError, "program not linked");
        return;
    }

    if (wasError()) {
        LOG_BEGIN(*this, err::Info);
        LOG_PUT_ERR(*this, getError(), "using program despite error");
        LOG_END(*this);
    }
    
    GL_CHECK(glUseProgram(self->program));
}

bool ShaderProgram::replaceWith(ShaderProgram& new_prog) {
    if (&new_prog != this) {
        ASSERT(&new_prog.self->sm == &self->sm);

        if (new_prog.self->program != 0 && new_prog.getError() == ShaderProgramError::NoError) {
            reset();
            self->program = new_prog.self->program;
            this->lastError = new_prog.lastError;
            self->shaders = new_prog.self->shaders;
            new_prog.self->program = 0;
            new_prog.reset();
            return true;
        } else {
            LOG_BEGIN(*this, err::Info);
            LOG_PUT_ERR(*this, new_prog.getError(), "replaceWith failed");
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
        pushError(ShaderProgramError::UniformNotKnown);
    return loc;
}

bool ShaderProgram::validate(bool printLogOnError) {
    bool ok = false;
    
    if (self->program == 0) {
        pushError(ShaderProgramError::APIError);
        goto ret;
    }

    GL_CHECK(glValidateProgram(self->program));
    GLint valid;
    GL_CHECK(glGetProgramiv(self->program, GL_VALIDATE_STATUS, &valid));
    
    if (valid == GL_FALSE)
        pushError(ShaderProgramError::ValidationFailed);
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
