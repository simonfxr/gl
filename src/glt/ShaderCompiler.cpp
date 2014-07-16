#include <stack>
#include <queue>
#include <algorithm>

#include "glt/ShaderCompiler.hpp"
#include "glt/utils.hpp"
#include "glt/GLSLPreprocessor.hpp"
#include "glt/GLObject.hpp"

#include "sys/fs.hpp"
#include "sys/measure.hpp"
#include "sys/io/Stream.hpp"

template <>
struct LogTraits<glt::CompileState> {
    static err::LogDestination getDestination(const glt::CompileState& cstate) {
        return err::LogDestination(err::Info, const_cast<glt::CompileState&>(cstate).compiler.shaderManager.out());
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

#define NULL_SHADER_SOURCE Ref<ShaderSource>()
#define NULL_SHADER_OBJECT Ref<ShaderObject>()
#define NULL_SHADER_CACHE  Ref<ShaderCache>()

namespace {

std::string hash(const std::string&);

struct ShaderTypeMapping {
    const char *fileExtension;
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

bool compilePreprocessed(CompileState&, GLenum, const std::string&, GLSLPreprocessor&, GLShaderObject&);

void printShaderLog(GLShaderObject&, sys::io::OutStream& out);

bool translateShaderType(ShaderManager::ShaderType type, GLenum *gltype, const std::string& basename = "");

void initPreprocessor(ShaderCompiler&, GLSLPreprocessor&);

ReloadState includesNeedReload(const ShaderIncludes&);

struct StringShaderObject : public ShaderObject {
    StringShaderObject(const Ref<StringSource>& src, GLuint hndl) :
        ShaderObject(src, hndl) {}

    virtual ReloadState needsReload() FINAL OVERRIDE;
};

struct FileShaderObject : public ShaderObject {
    sys::fs::FileTime mtime;
    FileShaderObject(const Ref<FileSource>& src, sys::fs::FileTime _mtime, GLuint hndl) :
        ShaderObject(src, hndl), mtime(_mtime) {}

    virtual ReloadState needsReload() FINAL OVERRIDE;
};

struct CompileJobSource : public CompileJob {
    Ref<ShaderSource> _source;
    CompileJobSource(const Ref<ShaderSource>& src) :
        _source(src) {}

    virtual Ref<ShaderSource>& source() FINAL OVERRIDE;
    virtual Ref<ShaderObject> exec(CompileState&) FINAL OVERRIDE;
};

struct CompileJobObject : public CompileJob {
    Ref<ShaderObject> shaderObject;
    CompileJobObject(const Ref<ShaderObject>& so) :
        shaderObject(so) {}

    virtual Ref<ShaderSource>& source() FINAL OVERRIDE;
    virtual Ref<ShaderObject> exec(CompileState&) FINAL OVERRIDE;
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
    proc.out(m.out());

    if (m.shaderVersion() != 0) {
        sys::io::ByteStream glversdef;
        glversdef << "#version " << m.shaderVersion()
                  << (m.shaderProfile() == ShaderManager::CoreProfile
                      ? " core" : " compatibility")
                  << sys::io::endl;
        proc.appendString(glversdef.str());
    }

    proc.addDefines(compiler.shaderManager.globalDefines());
    proc.addDefines(compiler.defines);
}

ReloadState includesNeedReload(const ShaderIncludes& incs) {
    for (index i = 0; i < SIZE(incs.size()); ++i) {
        sys::fs::FileTime mtime;
        if (!sys::fs::modificationTime(incs[size_t(i)].first, &mtime))
            return ReloadFailed;
        if (mtime != incs[size_t(i)].second)
            return ReloadOutdated;
    }

    return ReloadUptodate;
}

bool compilePreprocessed(CompileState& cstate, GLenum shader_type, const std::string& name, GLSLPreprocessor& proc, GLShaderObject& shader) {

    GLsizei nsegments = GLsizei(proc.segments.size());
    const char **segments = &proc.segments[0];
    const GLint *segLengths = reinterpret_cast<const GLint *>(&proc.segLengths[0]);

    shader.ensure(shader_type);
    if (!shader.valid()) {
        COMPILER_ERR_MSG(cstate, ShaderCompilerError::OpenGLError, "couldnt create shader");
        return false;
    }

    LOG_BEGIN(cstate, err::Info);
    LOG_PUT(cstate, std::string("compiling ") + (name.empty() ? " <embedded code> " : name) + " ... ");

#if 0

    {
        sys::io::OutStream& out = cstate.compiler.shaderManager.out();
        out << sys::io::endl;
        out << "BEGIN SHADER SOURCE" << sys::io::endl;
        
        for (uint32 i = 0; i < nsegments; ++i)
            out << std::string(segments[i], segLengths[i]);

        out << "END SHADER SOURCE" << sys::io::endl;
    }

#endif
        
    GL_CALL(glShaderSource, *shader, nsegments, segments, segLengths);
    double wct;
    measure_time(wct, glCompileShader(*shader));
    GL_CHECK_ERRORS();

    GLint success;
    GL_CALL(glGetShaderiv, *shader, GL_COMPILE_STATUS, &success);
    bool ok = success == GL_TRUE;
    LOG_PUT(cstate, (ok ? "success" : "failed")) << " (" << (wct * 1000) << " ms)" << sys::io::endl;
    LOG_END(cstate);

    if ((!ok && LOG_LEVEL(cstate, err::Error)) || LOG_LEVEL(cstate, err::Info)) {
        LOG_BEGIN(cstate, ok ? err::Info : err::Error);
        printShaderLog(shader, LOG_DESTINATION(cstate));
        LOG_END(cstate);
    }
    
    if (!ok) {
        cstate.pushError(ShaderCompilerError::CompilationFailed);
        shader.release();
    }

    return shader.valid();
}

void printShaderLog(GLShaderObject& shader, sys::io::OutStream& out) {
    GLint log_len;
    GL_CALL(glGetShaderiv, *shader, GL_INFO_LOG_LENGTH, &log_len);
    
    if (log_len > 0) {
        
        GLchar *log = new GLchar[size_t(log_len)];
        GL_CALL(glGetShaderInfoLog, *shader, log_len, NULL, log);

        GLchar *logBegin = log;
        while (logBegin < log + log_len - 1 && isspace(*logBegin))
            ++logBegin;

        GLchar *end = log + log_len - 1;
        while (end > logBegin && isspace(end[-1]))
            --end;
        *end = 0;
        const char *log_msg = log;

        if (logBegin == end)  {
            out << "shader compile log empty" << sys::io::endl;
        } else {
            out << "shader compile log: " << sys::io::endl
                << log_msg << sys::io::endl
                << "end compile log" << sys::io::endl;
        }
        
        delete[] log;
    }
}

bool translateShaderType(ShaderManager::ShaderType type, GLenum *gltype, const std::string& basename) {

    if (type == ShaderManager::GuessShaderType && !ShaderCompiler::guessShaderType(basename, &type))
        return false;
    
    for (index i = 0; i < ARRAY_LENGTH(shaderTypeMappings); ++i) {
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
    
    sys::fs::FileTime current_mtime;
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
    ASSERT(_self.ptr() == this);
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

    if (!compilePreprocessed(cstate, gltype, "", preproc, so->handle))
        return NULL_SHADER_OBJECT;
    return so;
}

FileSource::FileSource(ShaderManager::ShaderType ty, const std::string& path) :
    ShaderSource(path, ty)
{
    ASSERT_MSG(sys::fs::isAbsolute(path), "path not absolute: " + path);
}

Ref<ShaderObject> FileSource::load(Ref<ShaderSource>& _self, CompileState& cstate) {
    ASSERT(_self.ptr() == this);
    Ref<FileSource>& self = _self.cast<FileSource>();
    GLenum gltype;

    const std::string& path = filePath();
    if (!translateShaderType(type, &gltype, path)) {
        COMPILER_ERR_MSG(cstate, ShaderCompilerError::InvalidShaderType, "unknown ShaderType");
        return NULL_SHADER_OBJECT;
    }

    sys::fs::FileTime mtime;
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

    if (!compilePreprocessed(cstate, gltype, so->source->key, preproc, so->handle))
        return NULL_SHADER_OBJECT;
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
        if (it != compiled.end()) {
            continue;
        }

        Ref<ShaderObject> so;
        bool cache_hit = false;
        if (flags & SC_LOOKUP_CACHE) {
            if ((so = compiler.cache->lookup(key))) {
                ASSERT(key == so->source->key);
                cache_hit = true;

                ASSERT(so);
                if (flags & SC_CHECK_OUTDATED) {
                    so = reload(so);
                }
            }
        }

        if (!cache_hit) {
            so = job->exec(*this);
        }

        if (!so) {
            this->pushError(ShaderCompilerError::CompilationFailed);
        } else {
            put(so);
            
            for (index i = 0; i < SIZE(so->dependencies.size()); ++i) {
                Ref<CompileJob> job = CompileJob::load(so->dependencies[size_t(i)]);
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
    Ref<ShaderObject> reloaded_so;

    switch (state) {
    case ReloadUptodate: reloaded_so = so; break;
    case ReloadOutdated: reloaded_so = load(so->source); break;
    case ReloadFailed: break;
    default: ASSERT_FAIL();
    }

    return reloaded_so;
}

ShaderObject::~ShaderObject() {
    Ref<ShaderCache> c(cache);
    if (c) 
        ShaderCache::remove(c, this);
}

void ShaderObject::linkCache(Ref<ShaderCache>& newcache) {
    ASSERT(newcache && !Ref<ShaderCache>(cache));
    cache = newcache.weak();
}

void ShaderObject::unlinkCache(Ref<ShaderCache>& newcache) {
    Ref<ShaderCache> curr(cache);
    ASSERT(newcache && curr);
    if (curr == newcache)
        cache = NULL_SHADER_CACHE.weak();
}

ShaderCache::~ShaderCache() {
    flush();
}

template <typename I>
struct PairRange {
    I begin_range;
    I end_range;

    PairRange(const I& b, const I& e) :
        begin_range(b), end_range(e) {}
    PairRange(const std::pair<I, I>& pair) :
        begin_range(pair.first), end_range(pair.second) {}

    I& begin() { return begin_range; }
    const I& begin() const { return begin_range; }
    
    I& end() { return end_range; }
    const I& end() const { return end_range; }
};

template <typename I>
PairRange<I> pair_range(const std::pair<I, I>& pair) {
    return PairRange<I>(pair);
}

Ref<ShaderObject> ShaderCache::lookup(const ShaderSourceKey& key) {

    int best_version = -1;
    WeakRef<ShaderObject> best_ref;

    for (auto& kv : pair_range(entries.equal_range(key))) {
        ShaderCacheEntry& ent = kv.second;
        if (ent.first > best_version) {
            best_version = ent.first;
            best_ref = ent.second;
        }
    }

    Ref<ShaderObject> so;
    bool found = best_version >= 0;
    if (found) {
        so = best_ref.strong();
        ASSERT(so);
        ASSERT(key == so->source->key);
    }
    
    checkValid();
    return so;
}

bool ShaderCache::put(Ref<ShaderCache>& cache, Ref<ShaderObject>& so) {
    ASSERT(cache && so);

    Ref<ShaderCache> curr_cache(so->cache);
    ASSERT(!curr_cache || curr_cache == cache);

    const ShaderSourceKey& key = so->source->key;
    int next_version = 1;
    bool already_present = false;

    for (auto& kv : pair_range(cache->entries.equal_range(key))) {
        const ShaderCacheEntry& ent = kv.second;

        if (so == ent.second) {
            already_present = true;
            break;
        }
        
        if (ent.first >= next_version)
            next_version = ent.first + 1;
    }

    if (!already_present) {
        ASSERT(!curr_cache);
        so->cache = cache.weak();
        ShaderCacheEntry ent = std::make_pair(next_version, so.weak());
        cache->entries.insert(std::make_pair(key, ent));
    }
    
    cache->checkValid();
    return !already_present;
}

bool ShaderCache::remove(Ref<ShaderCache>& cache, ShaderObject *so) {
    ASSERT(cache && so);
    ASSERT(cache == so->cache);

    bool removed = false;
    const ShaderSourceKey& key = so->source->key;
    auto range = pair_range(cache->entries.equal_range(key));
    for (auto it = range.begin(); it != range.end(); ++it) {
        const ShaderCacheEntry& ent = it->second;

        if (ent.second == so) {
            removed = true;
            cache->entries.erase(it);
            break;
        }
    }

    cache->checkValid();
    return removed;
}

void ShaderCache::flush() {
    entries.clear();
    checkValid();
}

void ShaderCache::checkValid() {
    for (auto& kv : entries) {
        Ref<ShaderObject> so(kv.second.second);
        ASSERT(so);
        ASSERT(so->source->key == kv.first);
    }
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
        if (ext.compare(shaderTypeMappings[i].fileExtension) == 0) {
            *res = shaderTypeMappings[i].type;
            return true;
        }
    }

    return false;
}
    
} // namespace glt
