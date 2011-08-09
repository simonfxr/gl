#include <stack>
#include <sstream>
#include <set>

#include "glt/ShaderCompiler.hpp"
#include "glt/utils.hpp"
#include "glt/GLSLPreprocessor.hpp"

#include "sys/fs/fs.hpp"

template <>
struct LogTraits<glt::ShaderCompiler> {
    static err::LogDestination getDestination(const glt::ShaderCompiler& comp) {
        return err::LogDestination(err::Info, const_cast<glt::ShaderCompiler&>(comp).shaderManager.err());
    }
};

#define COMPILER_ERR_MSG(comp, ec, msg) LOG_RAISE(comp, ec, ::err::Error, msg)
#define COMPILER_MSG(comp, msg) (LOG_BEGIN(comp, ::err::Error), LOG_PUT(comp, msg), LOG_END(comp))

namespace glt {

namespace {

enum IncludesInfo {
    IncludesUnchanged,
    IncludesOutdated,
    IncludesNotFound
};

const Ref<ShaderSource> NULL_SHADER_SOURCE;
const Ref<ShaderObjectGraph> NULL_SHADER_GRAPH;
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

struct CompileState {
    ShaderCompiler& compiler;
    const ShaderCompileFlags flags;
    
    CompileState(ShaderCompiler& comp, ShaderCompileFlags flgs) :
        compiler(comp), flags(flgs) {}

    static GLuint compile(ShaderCompiler&, GLenum, const std::string&, GLSLPreprocessor&);

    static void printShaderLog(GLuint, std::ostream& out);
    
    static bool translateShaderType(ShaderManager::ShaderType type, GLenum *gltype, const std::string& basename = "");

    static void initPreprocessor(ShaderCompiler&, GLSLPreprocessor&);

    static IncludesInfo includesOutdated(const ShaderIncludes&);

    Ref<ShaderObjectGraph> loadRec(ShaderObjectRoots&, Ref<ShaderObject>&);
    
    Ref<ShaderObjectGraph> reloadRec(ShaderObjectRoots&, bool *, Ref<ShaderObjectGraph>&);

    Ref<ShaderObjectGraph> loadFile(ShaderObjectRoots&, ShaderManager::ShaderType, const std::string&);
};

struct StringSource : public ShaderSource {
    std::string code;
    StringSource(ShaderManager::ShaderType ty, const std::string& _code) :
        ShaderSource(hash(_code), ty), code(_code) {}

    ShaderObject *load(ShaderCompiler&, ShaderCompileFlags);
};

struct FileSource : public ShaderSource {
    FileSource(ShaderManager::ShaderType ty, const std::string& path) :
        ShaderSource(path, ty) {}
    
    const std::string& filePath() const { return this->key; }

    ShaderObject *load(ShaderCompiler&, ShaderCompileFlags);
    
    static ShaderObject *loadFile(ShaderCompiler&, ShaderManager::ShaderType, const std::string&, bool, ShaderCompileFlags);
};

struct StringShaderObject : public ShaderObject {
    StringShaderObject(const Ref<StringSource>& src, GLuint hndl) :
        ShaderObject(src.cast<ShaderSource>(), hndl) {}

    ShaderObject *reload(ShaderCompiler&, ShaderCompileFlags);
};

struct FileShaderObject : public ShaderObject {
    sys::fs::ModificationTime mtime;
    FileShaderObject(const Ref<FileSource>& src, sys::fs::ModificationTime _mtime, GLuint hndl) :
        ShaderObject(src.cast<ShaderSource>(), hndl), mtime(_mtime) {}
    
    ShaderObject *reload(ShaderCompiler&, ShaderCompileFlags);
};

void CompileState::initPreprocessor(ShaderCompiler& compiler, GLSLPreprocessor& proc) {
    ShaderManager& m = compiler.shaderManager;

    if (m.shaderVersion() != 0) {
        std::ostringstream glversdef;
        glversdef << "#version " << m.shaderVersion()
                  << (m.shaderProfile() == ShaderManager::CoreProfile
                      ? "core" : "compatibility")
                  << std::endl;
        proc.appendString(glversdef.str());
    }

    proc.addDefines(compiler.shaderManager.globalDefines());
    proc.addDefines(compiler.defines);
}

IncludesInfo CompileState::includesOutdated(const ShaderIncludes& incs) {
    for (index_t i = 0; i < incs.size(); ++i) {
        sys::fs::ModificationTime mtime;
        if (!sys::fs::modificationTime(incs[i].first, &mtime))
            return IncludesNotFound;
        if (mtime != incs[i].second)
            return IncludesOutdated;
    }

    return IncludesUnchanged;
}

GLuint CompileState::compile(ShaderCompiler& compiler, GLenum shader_type, const std::string& name, GLSLPreprocessor& proc) {

    uint32 nsegments = proc.segments.size();
    const char **segments = &proc.segments[0];
    const GLint *segLengths = reinterpret_cast<const GLint *>(&proc.segLengths[0]);
    
    GLuint shader;
    GL_CHECK(shader = glCreateShader(shader_type));
    if (shader == 0) {
        COMPILER_ERR_MSG(compiler, ShaderCompiler::OpenGLError, "couldnt create shader");
        return 0;
    }

    LOG_BEGIN(compiler, err::Info);
    LOG_PUT(compiler, std::string("compiling ") + (name.empty() ? " embedded code " : name) + " ... ");
        
    GL_CHECK(glShaderSource(shader, nsegments, segments, segLengths));
    GL_CHECK(glCompileShader(shader));

    GLint success;
    GL_CHECK(glGetShaderiv(shader, GL_COMPILE_STATUS, &success));
    bool ok = success == GL_TRUE;
    LOG_PUT(compiler, (ok ? "success" : "failed"));
    LOG_END(compiler);

    if ((!ok && LOG_LEVEL(compiler, err::Error)) || LOG_LEVEL(compiler, err::Info)) {
        LOG_BEGIN(compiler, ok ? err::Info : err::Error);
        printShaderLog(shader, LOG_DESTINATION(compiler));
        LOG_END(compiler);
    }
    
    if (!ok) {
        compiler.pushError(ShaderCompiler::CompilationFailed);
        GL_CHECK(glDeleteShader(shader));
        shader = 0;
    }

    return shader;
}

void CompileState::printShaderLog(GLuint shader, std::ostream& out) {
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

bool CompileState::translateShaderType(ShaderManager::ShaderType type, GLenum *gltype, const std::string& basename) {

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

Ref<ShaderObjectGraph> CompileState::loadRec(ShaderObjectRoots& visited, Ref<ShaderObject>& so) {
    Ref<ShaderObjectGraph> graph(new ShaderObjectGraph(so));
    visited[so->source->key] = graph;
    
    for (index_t i = 0; i < so->dependencies.size(); ++i) {
        Ref<ShaderObjectGraph> child(loadFile(visited, so->dependencies[i].first, so->dependencies[i].second));
        if (!child)
            return NULL_SHADER_GRAPH;

        graph->dependencies.insert(std::make_pair(child->root->source->key, child.weak()));
    }

    if (flags & SC_PUT_CACHE)
        ShaderCache::put(compiler.cache, graph);

    return graph;
}

Ref<ShaderObjectGraph> CompileState::loadFile(ShaderObjectRoots& visited, ShaderManager::ShaderType type, const std::string& file) {
    ShaderObjectRoots::iterator it = visited.find(file);

    if (it != visited.end())
        return it->second;

    if (flags & SC_LOOKUP_CACHE) {
        Ref<ShaderObjectGraph> cached;
        if (compiler.cache->lookup(&cached, file)) {
            if (flags & SC_RECURSIVE_RECOMPILE) {
                bool outdated;
                cached = reloadRec(visited, &outdated, cached);
            } else {
                visited[cached->root->source->key] = cached;
            }

            return cached;
        }
    }        

    ShaderObject *so = FileSource::loadFile(compiler, type, file, true, flags);
    if (so == 0)
        return NULL_SHADER_GRAPH;

    Ref<ShaderObject> soref(so);
    return loadRec(visited, soref);
}

Ref<ShaderObjectGraph> CompileState::reloadRec(ShaderObjectRoots& visited, bool *outdated, Ref<ShaderObjectGraph>& graph) {

    ShaderObjectRoots::iterator it = visited.find(graph->root->source->key);
    if (it != visited.end())
        return it->second;

    ShaderObject *newroot = graph->root->reload(compiler, flags);
    if (newroot == 0)
        return NULL_SHADER_GRAPH;

    Ref<ShaderObjectGraph> newgraph(new ShaderObjectGraph(makeRef(newroot)));
    *outdated = *outdated || graph->root != newroot;
    visited[newgraph->root->source->key] = newgraph;

    for (ShaderObjects::iterator it = graph->dependencies.begin();
         it != graph->dependencies.end(); ++it) {

        RefValue<ShaderObjectGraph> childref;
        if (!it->second.unweak(&childref)) { // cant happen, as ShaderObjectRoots keeps it alive
            WARN("dead childref");
            return NULL_SHADER_GRAPH;
        }

        Ref<ShaderObjectGraph> child(childref);
        Ref<ShaderObjectGraph> newchild(reloadRec(visited, outdated, child));
        if (!newchild)
            return NULL_SHADER_GRAPH;
        
        newgraph->dependencies.insert(std::make_pair(newchild->root->source->key, newchild.weak()));
    }
    
    for (index_t i = 0; i < graph->dependencies.size(); ++i) {
     
    }

    if (flags & SC_PUT_CACHE)
        ShaderCache::put(compiler.cache, newgraph);

    return newgraph;
}

std::string hash(const std::string& source) {
    return source; // FIXME: implement proper hash
}

ShaderObject *StringSource::load(ShaderCompiler& compiler, ShaderCompileFlags) {
    GLenum gltype;

    if (!CompileState::translateShaderType(type, &gltype)) {
        COMPILER_ERR_MSG(compiler, ShaderCompiler::InvalidShaderType, "unknown ShaderType");
        return 0;
    }

    ShaderObject *so = new StringShaderObject(makeRef(new StringSource(type, code)), 0);
    GLSLPreprocessor preproc(compiler.shaderManager.shaderDirectories(), so->includes, so->dependencies);
    preproc.name("");
    CompileState::initPreprocessor(compiler, preproc);

    preproc.process(code);

    if (preproc.wasError()) {
        delete so;
        return 0;
    }

    GLuint shader = CompileState::compile(compiler, gltype, "", preproc);
    
    if (shader == 0) {
        delete so;
        return 0;
    }
    
    so->handle = shader;
    return so;
}

ShaderObject *FileSource::load(ShaderCompiler& comp, ShaderCompileFlags flags) {
    return loadFile(comp, type, filePath(), true, flags);
}

ShaderObject *FileSource::loadFile(ShaderCompiler& compiler, ShaderManager::ShaderType type, const std::string& path0, bool absolute, ShaderCompileFlags) {
    GLenum gltype;

    std::string path = path0;

    if (!CompileState::translateShaderType(type, &gltype, path)) {
        COMPILER_ERR_MSG(compiler, ShaderCompiler::InvalidShaderType, "unknown ShaderType");
        return 0;
    }

    if (!absolute) {
        path = sys::fs::lookup(compiler.shaderManager.shaderDirectories(), path);
        if (path.empty()) {
            COMPILER_ERR_MSG(compiler, ShaderCompiler::FileNotInShaderPath, "file not found");
            return 0;
        }
    }

    FileShaderObject *so = new FileShaderObject(makeRef(new FileSource(type, path)), sys::fs::MIN_MODIFICATION_TIME, 0);
    GLSLPreprocessor preproc(compiler.shaderManager.shaderDirectories(), so->includes, so->dependencies);
    CompileState::initPreprocessor(compiler, preproc);

    if (!preproc.processFileRecursively(path, &so->source->key, &so->mtime)) {
        delete so;
        return 0;
    }

    GLuint shader = CompileState::compile(compiler, gltype, "", preproc);
    
    if (shader == 0) {
        delete so;
        return 0;
    }
    
    so->handle = shader;
    return so;
}

ShaderObject *StringShaderObject::reload(ShaderCompiler& comp, ShaderCompileFlags flags) {
    
    switch (CompileState::includesOutdated(includes)) {
    case IncludesUnchanged: return this;
    case IncludesOutdated: break;
    case IncludesNotFound: return 0;            
    }

    return source->load(comp, flags);
}

ShaderObject *FileShaderObject::reload(ShaderCompiler& comp, ShaderCompileFlags flags) {
    Ref<FileSource> filesource = source.cast<FileSource>();
    ASSERT(filesource);
    
    sys::fs::ModificationTime current_mtime;
    if (!sys::fs::modificationTime(filesource->filePath(), &current_mtime))
        return 0;

    if (current_mtime == mtime) {
        switch (CompileState::includesOutdated(includes)) {
        case IncludesUnchanged: return this;
        case IncludesOutdated: break;
        case IncludesNotFound: return 0;            
        }        
    }

    return FileSource::loadFile(comp, filesource->type, filesource->filePath(), true, flags);
}

} // namespace anon

ShaderObjectGraph::~ShaderObjectGraph() {
    RefValue<ShaderCache> cacheref;
    if (cache && cache.unweak(&cacheref)) {
        Ref<ShaderCache> cache(cacheref);
        ShaderCache::remove(cache, this);
    }
}

void ShaderObjectGraph::linkCache(Ref<ShaderCache>& newcache) {
    ASSERT(newcache && !cache);
    cache = newcache.weak();
}

void ShaderObjectGraph::unlinkCache(Ref<ShaderCache>& newcache) {
    ASSERT(newcache && cache);
    cache = NULL_SHADER_CACHE.weak();
}

ShaderCache::~ShaderCache() {
    flush();
}

bool ShaderCache::lookup(Ref<ShaderObjectGraph> *graph, const ShaderSourceKey& key) {
    ShaderCacheEntries::iterator it = entries.find(key);
    if (it != entries.end())
        return it->second.unweak(graph);
    return false;
}

bool ShaderCache::put(Ref<ShaderCache>& cache, Ref<ShaderObjectGraph>& ent) {
    ASSERT(cache && ent);
    ent->linkCache(cache);
    return cache->entries.insert(std::make_pair(ent->root->source->key, ent.weak())).second;
}

bool ShaderCache::remove(Ref<ShaderCache>& cache, ShaderObjectGraph *ent) {
    ASSERT(cache && ent);
    ShaderCacheEntries::iterator it = cache->entries.find(ent->root->source->key);
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

bool ShaderCompiler::reloadRecursively(ShaderObjectRoots& roots, bool *outdated, ShaderCompileFlags flags) {
    CompileState cstate(*this, flags);
    ShaderObjectRoots newroots;
    *outdated = false;
    for (ShaderObjectRoots::iterator it = roots.begin();
         it != roots.end(); ++it) {
        if (!cstate.reloadRec(newroots, outdated, it->second))
            return false;
    }

    if (outdated)
        roots = newroots;
    return true;
}

bool ShaderCompiler::loadString(ShaderObjectRoots& roots, ShaderManager::ShaderType type, const std::string& code, ShaderCompileFlags flags) {
    Ref<ShaderSource> src(new StringSource(type, code));
    ShaderObject *so = src->load(*this, flags);
    
    if (so) {
        CompileState cstate(*this, flags);
        Ref<ShaderObject> soref(so);
        ShaderObjectRoots newroots(roots);
        
        if (cstate.loadRec(newroots, soref)) {
            roots = newroots;
            return true;
        }
    }

    return false;
}

bool ShaderCompiler::loadFile(ShaderObjectRoots& roots, ShaderManager::ShaderType type, const std::string& path, bool absolute, ShaderCompileFlags flags) {
    ShaderObject *so = FileSource::loadFile(*this, type, path, absolute, flags);

    if (so) {
        CompileState cstate(*this, flags);
        Ref<ShaderObject> soref(so);
        ShaderObjectRoots newroots(roots);
        
        if (cstate.loadRec(newroots, soref)) {
            roots = newroots;
            return true;
        }
    }

    return false;
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
