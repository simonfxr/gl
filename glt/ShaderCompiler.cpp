#include "glt/ShaderCompiler.hpp"
#include "glt/utils.hpp"
#include "sys/fs/fs.hpp"

#include <stack>
#include <stringstream>
#include <set>

namespace glt {

namespace {

const Ref<ShaderSource> NULL_SHADER_SOURCE;
const Ref<ShaderObjectGraph> NULL_SHADER_GRAPH;

typedef std::set<ShaderSourceKey> VisitedSet;

struct ShaderTypeMapping {
    std::string fileExtension;
    ShaderProgram::ShaderType type;
    GLenum glType;
};

const ShaderTypeMapping shaderTypeMappings[] = {
    { "frag", ShaderProgram::FragmentShader, GL_FRAGMENT_SHADER },
    { "vert", ShaderProgram::VertexShader, GL_VERTEX_SHADER },
    { "geom", ShaderProgram::GeometryShader, GL_GEOMETRY_SHADER },
    { "tctl", ShaderProgram::TesselationControl, GL_TESS_CONTROL_SHADER },
    { "tevl", ShaderProgram::TesselationEvaluation, GL_TESS_EVALUATION_SHADER }
};

struct CompileState {
    ShaderCompiler& compiler;
    const ShaderCompilerFlags flags;
    VisitedSet visited;
    
    CompileState(ShaderCompiler& comp, ShaderCompilerFlags flgs) :
        compiler(comp), flags(flgs) {}

    GLuint compile(GLenum, const std::string&, uint32, const char * const [], const GLint []);

    void printShaderLog(GLuint);
    
    bool translateShaderType(ShaderProgram::ShaderType type, GLenum *gltype, const std::string basename = "");

    void initPreprocessor(GLSLPreprocessor&);

    Ref<ShaderObjectGraph> loadRec(ShaderObjectRoots&, Ref<ShaderSource>&);
    
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
        ShaderSource(path, ty), mtime(_mtime) {}
    
    const std::string& filePath() { return this->key; }

    ShaderObject *load(ShaderCompiler&, ShaderCompileFlags);
    
    static ShaderObject *loadFile(ShaderCompiler&, ShaderManager::ShaderType, const std::string&, bool);
};

struct StringShaderObject : public ShaderObject {
    StringShaderObject(const Ref<StringSource>& src, GLuint hndl) :
        ShaderObject(src, hndl) {}

    ShaderObject *reload(ShaderCompiler&, ShaderCompileFlags);
};

struct FileShaderObject : public ShaderObject {
    sys::fs::MTime mtime;
    FileShaderObject(const Ref<FileSource>& src, sys::fs::MTime _mtime, GLuint hndl) :
        ShaderObject(src, hndl), mtime(_mtime) {}
    
    ShaderObject *reload(ShaderCompiler&, ShaderCompileFlags);
};

void CompileState::initPreprocessor(GLSLPreprocessor& proc) {
    ShaderManager& m = comp.shaderManager;

    std::ostringstream glversdef;
    glversdef << "#version " << vers
              << (compiler.shaderManager.shaderProfile() == ShaderManager::CoreProfile
                  ? "core" : "compatibility")
              << std::endl;
    
    proc.appendString(glversdef.str());
    proc.addDefines(compiler.shaderManager.globalDefines());
    proc.addDefines(compiler.defines);
}

GLuint CompileState::compile(GLenum type, const std::string& name, uint32 nseg, const char * const segs[], const GLint lens[]) {
    
    GLuint shader;
    GL_CHECK(shader = glCreateShader(shader_type));
    if (shader == 0) {
        COMPILER_ERR_MSG(compiler, OpenGLError, "couldnt create shader" << std::endl);
        return 0;
    }

    COMPILER_MSG(compiler, "compiling " << file << " ... ");
        
    GL_CHECK(glShaderSource(shader, nsegments, segments, segLengths));
    GL_CHECK(glCompileShader(shader));

    GLint success;
    GL_CHECK(glGetShaderiv(shader, GL_COMPILE_STATUS, &success));
    bool ok = success == GL_TRUE;
    COMPILER_MSG(compiler, (ok ? "success" : "failed"));

    if ((!ok && LOG_LEVEL(compiler, OnlyErrors)) || LOG_LEVEL(compiler, Info)) {
        COMPILER_MSG(compiler ", ");
        printShaderLog(shader);
    } else if (LOG_LEVEL(compiler, OnlyErrors)) {
        COMPILER_MSG(compiler, std::endl);
    }
    
    if (!ok) {
        COMPILER_ERR(compiler, CompilationFailed);
        GL_CHECK(glDeleteShader(shader));
        shader = 0;
    }

    return shader;
}

void CompileState::printShaderLog(GLuint shader) {

    std::ostream& out = compiler.shaderManager.err();
    
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

bool CompileState::translateShaderType(ShaderProgram::ShaderType type, GLenum *gltype, const std::string& basename) {

    if (type == GuessShaderType && !ShaderCompiler::guessShaderType(basename, &type))
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
    Ref<ShaderObjectGraph> graph(so);
    visited[so->key] = graph;
    
    for (index_t i = 0; i < so->dependencies.size(); ++i) {
        graph->dependencies.push_back(loadFile(visited, so->dependencies[i].first, so->dependencies[i].second));
        if (!graph->dependencies[i])
            return NULL_SHADER_GRAPH;
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
        if (compiler.cache.lookup(&cached, file)) {
            if (flags & SC_RECURSIVE_RECOMPILE) {
                bool outdated;
                cached = reloadRec(visited, &outdated, cached))
            } else {
                visited[cached->source->key] = cached;
            }

            return cached;
        }
    }        

    ShaderObject *so = FileSource::loadFile(compiler, type, file, true, flags);
    if (so == 0)
        return NULL_SHADER_GRAPH;

    Ref<ShaderObject> soref(so);
    return loadRec(visited, soref))
}

Ref<ShaderObjectGraph> CompileState::reloadRec(ShaderObjectRoots& visited, bool *outdated, Ref<ShaderObjectGraph>& graph) {

    ShaderObjectRoots::iterator it = visited.find(graph->root->key);
    if (it != visited.end())
        return it->second;

    ShaderObject *newroot = graph->root->reload(compiler, flags);
    if (newroot == 0)
        return NULL_SHADER_GRAPH;

    Ref<ShaderObjectGraph> newgraph(makeRef(newroot));
    outdated = outdated || root != newroot;
    visited[newgraph->root->key] = newgraph;

    for (index_t i = 0; i < graph->dependencies.size(); ++i) {
        newgraph->dependencies.push_back(reloadRec(visited, outdated, graph.dependencies[i]));
        if (!newgraph->dependencies[i])
            return NULL_SHADER_GRAPH;
    }

    if (flags & SC_PUT_CACHE)
        ShaderCache::put(compiler.cache, newgraph);

    return true;
}

std::string hash(const std::string& source) {
    return source; // FIXME: implement proper hash
}

ShaderObject *StringSource::load(ShaderCompiler&, ShaderCompileFlags) {
    GLenum gltype;

    if (!translateShaderType(type, &gltype)) {
        COMPILER_ERR(compiler, InvalidParameter, "unknown ShaderType");
        return false;
    }

    ShaderObject *so = new StringShaderObject(makeRef(new StringSource(type, string)));
    GLSLPreprocessor preproc(compiler.shaderManager.shaderDirectories(), so->includes, so->dependencies);
    preproc.name("");
    initPreprocessor(preproc);

    if (!preproc.processRecursively(string)) {
        delete so;
        return 0;
    }

    GLuint shader = compile(gltype, "", preproc.segments.size(), preproc.segments, preproc.segLengths);
    
    if (shader == 0) {
        delete so;
        return 0;
    }
    
    so->handle = shader;
    return so;
}

static ShaderObject *FileSource::loadFile(ShaderCompiler& compiler, ShaderManager::ShaderType type, const std::string& path, bool absolute, ShaderCompileFlags) {
    GLenum gltype;

    if (!translateShaderType(type, &gltype, path)) {
        COMPILER_ERR(compiler, CouldntGuessShaderType, "unknown ShaderType");
        return 0;
    }

    if (!absolute) {
        path = sys::fs::lookup(comp.shaderManager.shaderDirectories(), path);
        if (path.empty()) {
            COMPILER_ERR(compiler, FileNotInShaderPath, "file not found");
            return 0;
        }
    }

    ShaderObject *so = new FileShaderObject(makeRef(new FileSource(type, path)), sys::fs::MIN_MTIME, 0);
    GLSLPreprocessor preproc(compiler.shaderManager.shaderDirectories(), so->includes, so->dependencies);
    initPreprocessor(preproc);

    if (!preproc.processFileRecursively(string, &so->src->key, &so->mtime)) {
        delete so;
        return 0;
    }

    GLuint shader = compile(gltype, "", preproc.segments.size(), preproc.segments, preproc.segLengths);
    
    if (shader == 0) {
        delete so;
        return 0;
    }
    
    so->handle = shader;
    return so;
}

ShaderObject *StringShaderObject::reload(ShaderCompiler&, ShaderCompileFlags) {
    return this; // never outdated
}

ShaderObject *FileShaderObject::reload(ShaderCompiler& comp, ShaderCompileFlags flags) {
    Ref<FileSource> filesource = source.cast<FileSource>();
    ASSERT(filesource);
    
    sys::fs::MTime current_mtime;
    if (!sys::fs::getMTime(filesource->filePath(), &current_mtime))
        return 0;

    if (current_mtime == mtime)
        return this;

    return loadFile(comp, filesource->type, filesource->filePath(), true, flags);
}

} // namespace anon

ShaderObjectGraph::~ShaderObjectGraph() {
    RefValue<ShaderCache> cacheref;
    if (cache && cache.unweak(&cacheref)) {
        Ref<ShaderCache> cache(cacheref);
        ShaderCache::remove(cache, this);
    }
}

ShaderObjectGraph::linkCache(Ref<ShaderCache>& newcache) {
    ASSERT(newcache && !cache);
    cache = newcache.weak();
}

ShaderObjectGraph::unlinkCache(Ref<ShaderCache>& newcache) {
    ASSERT(newcache && cache);
    cache = 0;
}

ShaderCache::~ShaderCache() {
    flush();
}

bool ShaderCache::lookup(Ref<ShaderObjectGraph> *graph, const ShaderSourceKey& key) {
    ShaderCacheEntries::const_iterator it = entries.find(key);
    if (it != entries.end())
        return it->second.unweak(graph);
    return false;
}

bool ShaderCache::put(Ref<ShaderCache>& cache, Ref<ShaderObjectGraph>& ent) {
    ASSERT(cache && ent);
    ent->linkCache(cache);
    return entries.insert(std::make_pair(ent->source->key, ent.weak())).second;
}

bool ShaderCache::remove(Ref<ShaderCache>& cache, ShaderObjectGraph *ent) {
    ASSERT(cache && ent);
    ShaderCache::iterator it = cache->entries.find(ent->source->key);
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

bool ShaderCompiler::reloadRecursively(ShaderObjectCollection& coll, bool *outdated, ShaderCompileFlags flags) {
    Ref<ShaderCache> localCache(new ShaderCache);
    CompileState cstate(*this, flags, localCache);
    ShaderObjectRoots newroots;
    outdated = false;
    for (ShaderObjectRoots::const_iterator it = coll.roots.begin();
         it != coll.roots.end(); ++it) {
        if (!cstate.reloadRec(newroots, outdated, it->second))
            return false;
    }

    if (outdated)
        coll.roots = newroots;
    return true;
}

bool ShaderCompiler::loadString(ShaderObjectCollection& coll, ShaderManager::ShaderType type, const std::string& code, ShaderCompileFlags flags) {
    Ref<ShaderSource> src(new StringSource(type, code));
    ShadeObjectGraph *so = src->load(*this, flags);
    
    if (so) {
        Ref<ShaderCache> localCache(new ShaderCache);
        CompileState cstate(*this, flags, localCache);
        Ref<ShaderObjectGraph> soref(so);
        ShaderObjectRoots newroots(coll.roots);
        
        if (cstate.loadRec(newroots, soref)) {
            coll.roots = newroots;
            return true;
        }
    }

    return false;
}

bool ShaderCompiler::loadFile(ShaderObjectCollection& coll, ShaderManager::ShaderType type, const std::string& path, bool absolute, ShaderCompileFlags flags) {
    ShaderObjectGraph *so = FileSource::loadFile(*this, type, path, absolute, flags);

    if (so) {
        Ref<ShaderCache> localCache(new ShaderCache);
        CompileState cstate(*this, flags, localCache);
        Ref<ShaderObjectGraph> soref(so);

        ShaderObjectRoots newroots(coll.roots);
        
        if (cstate.loadRec(newroots, soref)) {
            coll.roots = newroots;
            return true;
        }
    }

    return false;
}
                          
static bool ShaderCompiler::guessShaderType(const std::string& path, ShaderManager::ShaderType *res) {
    ASSERT(res);
    
    std::string ext = sys::fs::getExtension(path);
    
    for (uint32 i = 0; i < ARRAY_LENGTH(shaderTypeMappings); ++i) {
        if (shaderTypeMappings[i].fileExtension.compare(ext) == 0) {
            *res = shaderTypeMappings[i].type;
            return true;
        }
    }

    return false;
}
    
} // namespace glt
