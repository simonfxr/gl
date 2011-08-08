#ifndef GLT_SHADER_COMPILER_HPP
#define GLT_SHADER_COMPILER_HPP

#include "opengl.h"
#include "data/Ref.hpp"
#include "sys/fs/fs.hpp"
#include "glt/ShaderManager.hpp"

#include <string>
#include <map>
#include <vector>

namespace glt {

struct ShaderSource;
struct ShaderObject;
struct ShaderObjectGraph;
struct ShaderCache;
struct ShaderObjectCollection;

typedef std::string ShaderSourceKey;

typedef std::map<ShaderSourceKey, WeakRef<ShaderObjectGraph> > ShaderObjects;
typedef std::map<ShaderSourceKey, WeakRef<ShaderObjectGraph> > ShaderCacheEntries;
typedef std::map<ShaderSourceKey, Ref<ShaderObjectGraph> > ShaderObjectRoots;
typedef std::map<std::string, std::string> PreprocessorDefinitions;
typedef std::vector<std::string> IncludePath;
typedef std::pair<std::string, sys::fs::MTime> > ShaderInclude;
typedef std::vector<ShaderInclude> ShaderIncludes;
typedef std::string ShaderSourceFilePath; // absolute path
typedef std::pair<ShaderProgram::ShaderType, ShaderSourceFilePath> ShaderDependency;
typedef std::vector<ShaderDependency> ShaderDependencies;

typedef uint32 ShaderCompileFlags;

struct ShaderSource {
    ShaderSourceKey key;
    ShaderManager::ShaderType type

    ShaderSource(const ShaderSourceKey& _key, ShaderManager::ShaderType ty) :
        key(_key), type(ty) {}
    
    virtual ~ShaderSource() {}
    
    virtual ShaderObject *load(ShaderCompiler&, ShaderCompileFlags) = 0;

private:
    ShaderSource(const ShaderSource&);
    ShaderSource& operator =(const ShaderSource);
};

struct ShaderObject {
    Ref<ShaderSource> source;
    GLuint handle;
    ShaderIncludes includes;
    ShaderDependencies dependencies;
    
    ShaderObject(const Ref<ShaderSource>& src, GLuint hndl) :
        source(src), handle(hndl), dependencies(), includes(), cache() {}

    virtual ~ShaderObject() {}
    
    virtual ShaderObject *reload(ShaderCompiler&, ShaderCompileFlags) = 0;
    
private:
    ShaderObject(const ShaderObject&);
    ShaderObject& operator =(const ShaderObject&);
};

struct ShaderObjectGraph {
    Ref<ShaderObject> root;
    WeakRef<ShaderCache> cache;
    ShaderObjects dependencies; // has to be non circular
                                // (guranteed by the gathering of dependencies in GLSLPreprocessor)

    ShaderObjectGraph(const Ref<ShaderObject>& _root) :
        root(_root), cache(), dependencies() {}

    ~ShaderObjectGraph();
    
    void linkCache(const Ref<ShaderCache>&);
    void unlinkCache(const Ref<ShaderCache>&);
};

struct ShaderCache {
    ShaderCacheEntries entries;

    ShaderCache() :
        entries() {}
    
    ~ShaderCache();

    bool lookup(Ref<ShaderObjectGraph> *, const ShaderSourceKey&);
    static bool put(Ref<ShaderCache>&, Ref<ShaderObjectGraph>&);
    static bool remove(Ref<ShaderCache>&, ShaderObjectGraph *);
    void flush();

private:
    ShaderCache(const ShaderCache&);
    ShaderCache& operator =(const ShaderCache&);
};

struct ShaderObjectCollection {
    ShaderObjectRoots roots;

    ShaderObjectCollection() :
        roots() {}
};

static const ShaderCompileFlags SC_LOOKUP_CACHE = 1;
static const ShaderCompileFlags SC_PUT_CACHE = 2;
static const ShaderCompileFlags SC_RECURSIVE_RECOMPILE = 3;
static const ShaderCompileFlags SC_CHECK_OUTDATED = 4; // only use cache if file didnt get changed

static const ShaderCompileFlags SC_DEFAULT
  = SC_LOOKUP_CACHE | SC_PUT_CACHE | SC_RECURSIVE_RECOMPILE | SC_CHECK_OUTDATED;

struct ShaderCompiler {
    glt::ShaderManager& shaderManager;
    PreprocessorDefinitions defines;
    Ref<ShaderCache> cache;
    Error lastError;

    enum Error {
        NoError,
        CouldntGuessShaderType,
        FileNotFound,
        FileNotInShaderPath,
        OpenGLError
    };

    ShaderCompiler(glt::ShaderManager& mng) :
        shaderManager(mng),
        defines(),
        cache(mng.globalShaderCache()),
        lastError(NoError) {}

    ~ShaderCompiler();

    Error getError() { return lastError; }

    void pushError(Error err) { if (lastError == NoError) lastError = err; }

    bool wasError() { return lastError != NoError; }

    Error clearError() { Error err = lastError; lastError = NoError; return err; }

    bool reloadRecursively(ShaderObjectCollection&, bool *, ShaderCompileFlags flags = SC_DEFAULT);

    bool loadString(ShaderObjectCollection&, ShaderManager::ShaderType, const std::string&, ShaderCompileFlags flags = SC_DEFAULT);

    bool loadFile(ShaderObjectCollection&, ShaderManager::ShaderType, const std::string&, bool, ShaderCompileFlags flags = SC_DEFAULT);
    
    static bool guessShaderType(const std::string& path, ShaderProgram::ShaderType *res);
private:
    
    ShaderCompiler(const ShaderCompiler&);
    ShaderCompiler& (const ShaderCompiler&);
};

} // namespace glt

#endif
