#include <stack>
#include <sstream>
#include <queue>

#include "glt/ShaderCompiler.hpp"
#include "glt/utils.hpp"
#include "glt/GLSLPreprocessor.hpp"

#include "sys/fs/fs.hpp"
#include "sys/measure.hpp"

template <>
struct LogTraits<glt::CompileState> {
    static err::LogDestination getDestination(const glt::CompileState& cstate) {
        return err::LogDestination(err::Info, const_cast<glt::CompileState&>(cstate).compiler.shaderManager.err());
    }
};

#define COMPILER_ERR_MSG(comp, ec, msg) LOG_RAISE(comp, ec, ::err::Error, msg)
#define COMPILER_MSG(comp, msg) (LOG_BEGIN(comp, ::err::Error), LOG_PUT(comp, msg), LOG_END(comp))

namespace glt {

namespace ShaderCompilerError {

std::string stringError(Type) {
    // FIXME: implement
    return "{shader compiler error}";
}

}

namespace {

const Ref<ShaderSource> NULL_SHADER_SOURCE;
const Ref<ShaderObject> NULL_SHADER_OBJECT;
Ref<ShaderCache> NULL_SHADER_CACHE;

std::string hash(const std::string&);

struct ShaderTypeMapping {
    std::string fileExtension;
    ShaderManager::ShaderType type;
    GLenum glType;
};

const ShaderTypeMapping shaderTypeMappings[] = {
    { "frag", ShaderManager::FragmentShader, GL_FRAGMENT_SHADER },
    { "vert", ShaderManager::VertexShader, GL_VERTEX_SHADER },
    { "geom", ShaderManager::GeometryShader, GL_GEOMETRY_SHADER },
    { "tctl", ShaderManager::TesselationControl, GL_TESS_CONTROL_SHADER },
    { "tevl", ShaderManager::TesselationEvaluation, GL_TESS_EVALUATION_SHADER }
};

GLuint compilePreprocessed(CompileState&, GLenum, const std::string&, GLSLPreprocessor&);

void printShaderLog(GLuint, std::ostream& out);

bool translateShaderType(ShaderManager::ShaderType type, GLenum *gltype, const std::string& basename = "");

void initPreprocessor(ShaderCompiler&, GLSLPreprocessor&);

ReloadState includesNeedReload(const ShaderIncludes&);

struct StringShaderObject : public ShaderObject {
    StringShaderObject(const Ref<StringSource>& src, GLuint hndl) :
        ShaderObject(src.cast<ShaderSource>(), hndl) {}

    ReloadState needsReload();
};

struct FileShaderObject : public ShaderObject {
    sys::fs::ModificationTime mtime;
    FileShaderObject(const Ref<FileSource>& src, sys::fs::ModificationTime _mtime, GLuint hndl) :
        ShaderObject(src.cast<ShaderSource>(), hndl), mtime(_mtime) {}

    ReloadState needsReload();
};

struct CompileJobSource : public CompileJob {
    Ref<ShaderSource> _source;
    CompileJobSource(const Ref<ShaderSource>& src) :
        _source(src) {}

    Ref<ShaderSource>& source();
    Ref<ShaderObject> exec(CompileState&);
};

struct CompileJobObject : public CompileJob {
    Ref<ShaderObject> shaderObject;
    CompileJobObject(const Ref<ShaderObject>& so) :
        shaderObject(so) {}

    Ref<ShaderSource>& source();
    Ref<ShaderObject> exec(CompileState&);
};

Ref<ShaderSource>& CompileJobSource::source() {
    return _source;
}

Ref<ShaderObject> CompileJobSource::exec(CompileState& cstate) {
    return cstate.load(_source);
}

Ref<ShaderSource>& CompileJobObject::source() {
    return shaderObject->source;
}

Ref<ShaderObject> CompileJobObject::exec(CompileState& cstate) {
    return cstate.reload(shaderObject);
}

void initPreprocessor(ShaderCompiler& compiler, GLSLPreprocessor& proc) {
    ShaderManager& m = compiler.shaderManager;
    proc.err(std::cerr);

    if (m.shaderVersion() != 0) {
        std::ostringstream glversdef;
        glversdef << "#version " << m.shaderVersion()
                  << (m.shaderProfile() == ShaderManager::CoreProfile
                      ? " core" : " compatibility")
                  << std::endl;
        proc.appendString(glversdef.str());
    }

    proc.addDefines(compiler.shaderManager.globalDefines());
    proc.addDefines(compiler.defines);
}

ReloadState includesNeedReload(const ShaderIncludes& incs) {
    for (index_t i = 0; i < incs.size(); ++i) {
        sys::fs::ModificationTime mtime;
        if (!sys::fs::modificationTime(incs[i].first, &mtime))
            return ReloadFailed;
        if (mtime != incs[i].second)
            return ReloadOutdated;
    }

    return ReloadUptodate;
}

GLuint compilePreprocessed(CompileState& cstate, GLenum shader_type, const std::string& name, GLSLPreprocessor& proc) {

    uint32 nsegments = proc.segments.size();
    const char **segments = &proc.segments[0];
    const GLint *segLengths = reinterpret_cast<const GLint *>(&proc.segLengths[0]);

    // std::cerr << "BEGIN SHADER SOURCE" << std::endl;

    // for (uint32 i = 0; i < nsegments; ++i) {
    //     std::cerr << std::string(segments[i], segLengths[i]);
    // }

    // std::cerr << "END SHADER SOURCE" << std::endl;
    
    GLuint shader;
    GL_CHECK(shader = glCreateShader(shader_type));
    if (shader == 0) {
        COMPILER_ERR_MSG(cstate, ShaderCompilerError::OpenGLError, "couldnt create shader");
        return 0;
    }

    LOG_BEGIN(cstate, err::Info);
    LOG_PUT(cstate, std::string("compiling ") + (name.empty() ? " embedded code " : name) + " ... ");
        
    GL_CHECK(glShaderSource(shader, nsegments, segments, segLengths));
    float wct;
    measure_time(wct, glCompileShader(shader));
    GL_CHECK_ERRORS();

    GLint success;
    GL_CHECK(glGetShaderiv(shader, GL_COMPILE_STATUS, &success));
    bool ok = success == GL_TRUE;
    LOG_PUT(cstate, (ok ? "success" : "failed")) << " (" << (wct * 1000) << " ms)" << std::endl;
    LOG_END(cstate);

    if ((!ok && LOG_LEVEL(cstate, err::Error)) || LOG_LEVEL(cstate, err::Info)) {
        LOG_BEGIN(cstate, ok ? err::Info : err::Error);
        printShaderLog(shader, LOG_DESTINATION(cstate));
        LOG_END(cstate);
    }
    
    if (!ok) {
        cstate.pushError(ShaderCompilerError::CompilationFailed);
        GL_CHECK(glDeleteShader(shader));
        shader = 0;
    }

    return shader;
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

bool translateShaderType(ShaderManager::ShaderType type, GLenum *gltype, const std::string& basename) {

    if (type == ShaderManager::GuessShaderType && !ShaderCompiler::guessShaderType(basename, &type))
        return false;
    
    for (index_t i = 0; i < ARRAY_LENGTH(shaderTypeMappings); ++i) {
        if (shaderTypeMappings[i].type == type) {
            *gltype = shaderTypeMappings[i].glType;
            return true;
        }
    }

    return false;
}

std::string hash(const std::string& source) {
    return source; // FIXME: implement proper hash
}

ReloadState StringShaderObject::needsReload() {
    return includesNeedReload(includes);
}

ReloadState FileShaderObject::needsReload() {
    Ref<FileSource>& filesource = source.cast<FileSource>();
    
    sys::fs::ModificationTime current_mtime;
    if (!sys::fs::modificationTime(filesource->filePath(), &current_mtime))
        return ReloadFailed;

    if (current_mtime != mtime)
        return ReloadOutdated;
        
    return includesNeedReload(includes);
}

} // namespace anon

StringSource::StringSource(ShaderManager::ShaderType type, const std::string& name) :
    ShaderSource(hash(name), type), code(name) {}

Ref<ShaderObject> StringSource::load(Ref<ShaderSource>& _self, CompileState& cstate) {
    ASSERT(_self == this);
    Ref<StringSource>& self = _self.cast<StringSource>();
    GLenum gltype;

    if (!translateShaderType(type, &gltype)) {
        COMPILER_ERR_MSG(cstate, ShaderCompilerError::InvalidShaderType, "unknown ShaderType");
        return NULL_SHADER_OBJECT;
    }

    Ref<ShaderObject> so(new StringShaderObject(self, 0));
    GLSLPreprocessor preproc(cstate.compiler.shaderManager.shaderDirectories(), so->includes, so->dependencies);
    preproc.name("");
    initPreprocessor(cstate.compiler, preproc);

    preproc.process(code);

    if (preproc.wasError())
        return NULL_SHADER_OBJECT;

    GLuint shader = compilePreprocessed(cstate, gltype, "", preproc);
    
    if (shader == 0)
        return NULL_SHADER_OBJECT;
    
    so->handle = shader;
    return so;
}

FileSource::FileSource(ShaderManager::ShaderType ty, const std::string& path) :
    ShaderSource(path, ty)
{
    ASSERT(sys::fs::isAbsolute(path));
}

Ref<ShaderObject> FileSource::load(Ref<ShaderSource>& _self, CompileState& cstate) {
    ASSERT(_self == this);
    Ref<FileSource>& self = _self.cast<FileSource>();
    GLenum gltype;

    const std::string& path = filePath();
    if (!translateShaderType(type, &gltype, path)) {
        COMPILER_ERR_MSG(cstate, ShaderCompilerError::InvalidShaderType, "unknown ShaderType");
        return NULL_SHADER_OBJECT;
    }

    sys::fs::ModificationTime mtime;
    if (!sys::fs::modificationTime(filePath(), &mtime)) {
        COMPILER_ERR_MSG(cstate, ShaderCompilerError::FileNotFound, "couldnt query mtime");
        return NULL_SHADER_OBJECT;
    }
    
    Ref<ShaderObject> so(new FileShaderObject(self, mtime, 0));

    GLSLPreprocessor preproc(cstate.compiler.shaderManager.shaderDirectories(), so->includes, so->dependencies);
    initPreprocessor(cstate.compiler, preproc);

    preproc.processFileRecursively(path);
    if (preproc.wasError())
        return NULL_SHADER_OBJECT;

    GLuint shader = compilePreprocessed(cstate, gltype, so->source->key, preproc);
    
    if (shader == 0)
        return NULL_SHADER_OBJECT;

    so->handle = shader;
    return so;
}

Ref<CompileJob> CompileJob::load(const Ref<ShaderSource>& src) {
    ASSERT(src);
    return Ref<CompileJob>(new CompileJobSource(src));
}

Ref<CompileJob> CompileJob::reload(const Ref<ShaderObject>& so) {
    ASSERT(so);
    return Ref<CompileJob>(new CompileJobObject(so));
}

void CompileState::enqueue(Ref<CompileJob>& job) {
    ASSERT(job);

    Ref<ShaderSource>& source = job->source();
    ASSERT(source);
    if (inQueue.count(source->key) > 0)
        return;

    toCompile.push(job);
}

void CompileState::compileAll() {
    
    for (; !wasError() && !toCompile.empty(); toCompile.pop()) {
        
        Ref<CompileJob>& job = toCompile.front();
        const ShaderSourceKey& key = job->source()->key;
        
        ShaderObjects::const_iterator it = compiled.find(key);
        if (it != compiled.end())
            continue;

        Ref<ShaderObject> so;
        bool cache_hit = false;
        if (flags & SC_LOOKUP_CACHE) {
            if (compiler.cache->lookup(&so, key)) {
                cache_hit = true;

                ASSERT(so);
                if (flags & SC_CHECK_OUTDATED)
                    so = reload(so);
            }
        }

        if (!cache_hit)
            so = job->exec(*this);

        if (!so) {
            this->pushError(ShaderCompilerError::CompilationFailed);
        } else {
            put(so);
            
            for (index_t i = 0; i < so->dependencies.size(); ++i) {
                Ref<CompileJob> job = CompileJob::load(so->dependencies[i]);
                enqueue(job);
            }
        }
    }
}

void CompileState::put(const Ref<ShaderObject>& so) {
    ASSERT(so);
    ASSERT(compiled.count(so->source->key) == 0);

    compiled.insert(std::make_pair(so->source->key, so));

    if (flags & SC_PUT_CACHE) {
        Ref<ShaderObject> so_mut(so);
        ShaderCache::put(compiler.cache, so_mut);
    }
}

Ref<ShaderObject> CompileState::load(Ref<ShaderSource>& src) {
    ASSERT(src);
    return src->load(src, *this);
}

Ref<ShaderObject> CompileState::reload(Ref<ShaderObject>& so) {
    ASSERT(so);
    ReloadState state = so->needsReload();
    switch (state) {
    case ReloadUptodate: return so;
    case ReloadOutdated: return load(so->source);
    case ReloadFailed: return NULL_SHADER_OBJECT;
    }

    ASSERT_FAIL();
}

ShaderObject::~ShaderObject() {
    RefValue<ShaderCache> cref;
    if (cache && cache.unweak(&cref)) {
        Ref<ShaderCache> c(cref);
        ShaderCache::remove(c, this);
    }
}

void ShaderObject::linkCache(Ref<ShaderCache>& newcache) {
    ASSERT(newcache && !cache);
    cache = newcache.weak();
}

void ShaderObject::unlinkCache(Ref<ShaderCache>& newcache) {
    ASSERT(newcache && cache);
    if (cache.same(newcache))
        cache = NULL_SHADER_CACHE.weak();
}

ShaderCache::~ShaderCache() {
    flush();
}

bool ShaderCache::lookup(Ref<ShaderObject> *ent, const ShaderSourceKey& key) {
    ShaderCacheEntries::iterator it = entries.find(key);
    if (it != entries.end()) {
        bool alive = it->second.unweak(ent);
        ASSERT(alive);
        return true;
    }
    return false;
}

bool ShaderCache::put(Ref<ShaderCache>& cache, Ref<ShaderObject>& ent) {
    ASSERT(cache && ent);
    if (ent->cache.same(cache))
        return false;
    ent->linkCache(cache);
    return cache->entries.insert(std::make_pair(ent->source->key, ent.weak())).second;
}

bool ShaderCache::remove(Ref<ShaderCache>& cache, ShaderObject *ent) {
    ASSERT(cache && ent);
    ShaderCacheEntries::iterator it = cache->entries.find(ent->source->key);
    if (it != cache->entries.end() && it->second == ent) {
        ent->unlinkCache(cache);
        cache->entries.erase(it);
        return true;
    }

    return false;
}

void ShaderCache::flush() {
    entries.clear();
}

ShaderCompiler::~ShaderCompiler() {
    
}

void ShaderCompiler::init() {
    cache = shaderManager.globalShaderCache();
}

bool ShaderCompiler::guessShaderType(const std::string& path, ShaderManager::ShaderType *res) {
    ASSERT(res);
    
    std::string ext = sys::fs::extension(path);
    
    for (uint32 i = 0; i < ARRAY_LENGTH(shaderTypeMappings); ++i) {
        if (shaderTypeMappings[i].fileExtension.compare(ext) == 0) {
            *res = shaderTypeMappings[i].type;
            return true;
        }
    }

    return false;
}
    
} // namespace glt
