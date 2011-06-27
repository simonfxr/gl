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
#include "error/error.hpp"
#include "glt/utils.hpp"
#include "glt/Preprocessor.hpp"
#include "glt/GLSLPreprocessor.hpp"

#define LOG_LEVEL(data, lvl) ((data)->sm.verbosity() >= ShaderManager::lvl)

#define LOG(data, lvl, msg) do { if (LOG_LEVEL(data, lvl)) { (data)->sm.err() << msg; } } while (0)

#define LOG_INFO(data, msg) LOG(data, Info, msg)
#define LOG_ERR(data, msg) LOG(data, OnlyErrors, msg)

namespace glt {

typedef ShaderManager::CachedShaderObject CachedShaderObject;

typedef std::map<std::string, Ref<CachedShaderObject> > ShaderMap;

typedef std::map<std::string, GLuint> Attributes;

struct ShaderProgram::Data {
    Error last_error;
    GLuint program;
    ShaderManager& sm;
    ShaderMap shaders; // contains only root shader files and during compilation markers to prevent endless recursive compiles
    Attributes attrs;
    CachedShaderObject *parentSO;
    bool linked;

    Data(ShaderManager& _sm) :
        sm(_sm)
        {}

    void push_error(ShaderProgram::Error err) {
        if (last_error == NoError)
            last_error = err;
    }

    bool addShader(ShaderType type, const std::string& file, uint32 nsegs, const char *segments[], const GLint segLengths[], GLuint *handle = 0);
    bool createProgram();
    bool attachShaderObjects(std::set<const Ref<CachedShaderObject> *>& added, const Ref<CachedShaderObject>& cached);
};

ShaderProgram::ShaderProgram(ShaderManager& sm) :
    self(new Data(sm))
{
    self->last_error = NoError;
    self->program = 0;
    self->parentSO = 0;
    self->linked = false;
}

ShaderProgram::~ShaderProgram() {
    reset();
    delete self;
}

void ShaderProgram::reset() {
    GL_CHECK(glDeleteProgram(self->program));
    self->program = 0;
    self->last_error = NoError;
    self->shaders.clear();
}

bool ShaderProgram::reload() {
    ShaderProgram new_prog(self->sm);

    bool outdated = false;
    {
        ShaderMap::const_iterator it = self->shaders.begin();
        for (; it != self->shaders.end(); ++it) {
            if (!new_prog.addShaderFile(it->first, true))
                return false;
            outdated = it->second != new_prog.self->shaders[it->first];
        }
    }

    Attributes::const_iterator it = self->attrs.begin();
    for (; it != self->attrs.end(); ++it)
        if (!bindAttribute(it->first, it->second))
            return false;

    if (outdated && new_prog.tryLink()) {
       return replaceWith(new_prog);
    } else {
        return false;
    }
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
    ASSERT(self->createProgram());
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
    GL_CHECK(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len));

    if (log_len > 0) {
            
        GLchar *log = new GLchar[log_len];

        GLchar *logBegin = log;
        while (logBegin < log + log_len - 1 && isspace(*logBegin))
            ++logBegin;

        if (logBegin == log + log_len - 1)  {
            out << "shader compile log empty" << std::endl;
        } else {
            out << "shader compile log: " << std::endl;
            GL_CHECK(glGetShaderInfoLog(shader, log_len, NULL, log));
            
            out << log << std::endl
                << "end compile log" << std::endl;
        }
        
        delete[] log;
    }
}

void printProgramLog(GLuint program, std::ostream& out) {

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

} // namespace anon

bool ShaderProgram::Data::createProgram() {
    if (program == 0) {
        GL_CHECK(program = glCreateProgram());
        if (program == 0) {
            LOG_ERR(this, "couldnt create program" << std::endl);
            push_error(OpenGLError);
            return false;
        }
    }
    return true;
}

bool ShaderProgram::Data::attachShaderObjects(std::set<const Ref<CachedShaderObject> *>& added, const Ref<CachedShaderObject>& cached) {
    DEBUG_ASSERT(cached.ptr() != 0);
    if (added.count(&cached) == 0) {
        added.insert(&cached);
        
        for (uint32 i = 0; i < cached->deps.size(); ++i)
            if (!attachShaderObjects(added, cached->deps[i]))
                return false;
        
        ASSERT(cached->so.handle != 0);
        GL_CHECK(glAttachShader(program, cached->so.handle));
    }        
    return true;
}

bool ShaderProgram::Data::addShader(ShaderType type, const std::string& file, uint32 nsegments, const char *segments[], const GLint segLengths[], GLuint *handle) {

    GLenum shader_type;
    if (!translateShaderType(type, &shader_type)) {
        LOG_ERR(this, "unknown shader type" << std::endl);
        push_error(APIError);
        return false;
    }

    if (!createProgram())
        return false;

    linked = false;

    GLuint shader;
    GL_CHECK(shader = glCreateShader(shader_type));
    if (shader == 0) {
        LOG_ERR(this, "couldnt create shader" << std::endl);
        push_error(OpenGLError);
        return false;
    }

    LOG_ERR(this, "compiling " << file << " ... ");
        
    GL_CHECK(glShaderSource(shader, nsegments, segments, segLengths));
    GL_CHECK(glCompileShader(shader));

    GLint success;
    GL_CHECK(glGetShaderiv(shader, GL_COMPILE_STATUS, &success));
    bool ok = success == GL_TRUE;
    LOG_ERR(this, (ok ? "success" : "failed"));

    if ((!ok && LOG_LEVEL(this, OnlyErrors)) || LOG_LEVEL(this, Info)) {
        sm.err() << ", ";
        printShaderLog(shader, sm.err());
    } else if (LOG_LEVEL(this, OnlyErrors)) {
        sm.err() << std::endl;
    }
    
    if (!ok)
        push_error(CompilationFailed);

    if (ok && handle == 0)
        GL_CHECK(glAttachShader(program, shader));

    if (!ok || handle == 0)
        GL_CHECK(glDeleteShader(shader));

    if (handle != 0)
        *handle = ok ? shader : 0;

    return ok;
}

bool ShaderProgram::addShaderSrc(ShaderType type, const std::string& src) {
    const GLchar *str = src.c_str();
    GLsizei length = src.length();
    return self->addShader(type, "<unknown>", 1, &str, &length);
}

bool ShaderProgram::addShaderFile(const std::string& file, bool absolute) {
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
        LOG_ERR(self, "couldnt guess shader type based on file name" << std::endl);
        self->push_error(CompilationFailed);
        return false;
    }

    return addShaderFile(type, file, absolute);
}

bool ShaderProgram::addShaderFile(ShaderType type, const std::string& file, bool absolute) {

    DependencyHandler depHandler;
    bool success = false;
    std::string realname;
    CachedShaderObject *myParentSO = self->parentSO;
    Ref<CachedShaderObject> cacheEntry;

    {
        Preprocessor proc;
        IncludeHandler incHandler;
        ShaderContents shadersrc;

        depHandler.patches = &incHandler.includes;
    
        proc.installHandler("include", incHandler);
        proc.installHandler("need", depHandler);
        proc.err(self->sm.err());

        realname = absolute ? file : self->sm.lookupPath(file);

        if (realname.empty()) {
            LOG_ERR(self, "couldnt find file in path: " << file << std::endl);
            return false;
        } else if (self->shaders.count(realname) > 0) {
            return true;
        }

        sys::fs::MTime mtime = sys::fs::getMTime(realname);
        {
            cacheEntry = self->sm.lookupShaderObject(realname, mtime);
            if (cacheEntry.ptr() != 0) {
                DEBUG_ASSERT(cacheEntry->key == realname);
                success = true;
                goto ret_add;
            }
        }

        self->shaders[realname] = ShaderManager::EMPTY_CACHE_ENTRY; // mark as added (prevent recursive adds)
        cacheEntry = new CachedShaderObject(self->sm, realname, mtime);

        uint32 shader_vers = self->sm.shaderVersion();
        std::string versionStr = "";

        if (shader_vers != 0) {
            std::ostringstream svers;
            svers << "#version " << shader_vers;
            if (self->sm.shaderProfile() == ShaderManager::CompatibilityProfile) {
                svers << " compatibility";
            }
            versionStr = svers.str();
        }

        if (!preprocess(self->sm, proc, realname, &incHandler.includes, cacheEntry->incs, versionStr, shadersrc)) {
            LOG_ERR(self, "couldnt process shader file" << std::endl);
            self->push_error(CompilationFailed);
            goto ret;
        }

        {
            uint32 nsegments = shadersrc.segments.size();
            const uint32 *segLengths = &shadersrc.segLengths[0];
            const char **segments = &shadersrc.segments[0];
            GLuint handle = 0;
            if (!self->addShader(type, realname, nsegments, segments, (GLint *) segLengths, &handle))
                goto ret;

            cacheEntry->so.handle = handle;
            self->sm.cacheShaderObject(cacheEntry);
        }

        self->parentSO = cacheEntry.ptr();
    }

    for (uint32 i = 0; i < depHandler.deps.size(); ++i)
        if (!addShaderFile(depHandler.deps[i]))
            goto ret_pop;

    success = true;

ret_pop:
    self->parentSO = myParentSO;

ret_add:

    if (myParentSO == 0) { // root dependency
        DEBUG_ASSERT(cacheEntry.ptr() != 0);
        self->shaders[cacheEntry->key] = cacheEntry;
    } else {
        myParentSO->deps.push_back(cacheEntry);
    }
    
ret:

    if (myParentSO != 0)
        self->shaders.erase(realname);

    return success;
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

    if (!self->createProgram())
        return false;

    if (self->linked)
        return true;

    std::set<const Ref<CachedShaderObject> *> added;
    for (ShaderMap::const_iterator it = self->shaders.begin(); it != self->shaders.end(); ++it)
        if (!self->attachShaderObjects(added, it->second))
            return false;

    LOG_ERR(self, "linking ... ");

    GL_CHECK(glLinkProgram(self->program));

    GLint success;
    GL_CHECK(glGetProgramiv(self->program, GL_LINK_STATUS, &success));
    bool ok = success == GL_TRUE;
    LOG_ERR(self, (ok ? "success" : "failed"));

    if (!ok)
        self->push_error(LinkageFailed);

    if ((!ok && LOG_LEVEL(self, OnlyErrors)) || LOG_LEVEL(self, Info)) {
        self->sm.err() << ", ";
        printProgramLog(self->program, self->sm.err());
    } else if (LOG_LEVEL(self, OnlyErrors)) {
        self->sm.err() << std::endl;
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

    if (&new_prog != this) {
        ASSERT(&new_prog.self->sm == &self->sm);

        if (new_prog.self->program != 0 && new_prog.self->last_error == NoError) {
            reset();
            self->program = new_prog.self->program;
            self->last_error = NoError;
            self->shaders = new_prog.self->shaders;
            new_prog.self->program = 0;
            new_prog.reset();
            return true;
        } else {
            new_prog.printError(self->sm.err());
            return false;
        }
    } else {
        return true;
    }
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
    bool ok = false;
    
    if (self->program == 0) {
        self->push_error(APIError);
        goto ret;
    }

    GL_CHECK(glValidateProgram(self->program));
    GLint valid;
    GL_CHECK(glGetProgramiv(self->program, GL_VALIDATE_STATUS, &valid));
    
    if (valid == GL_FALSE)
        self->push_error(ValidationFailed);
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


} // namespace glt
