#ifndef GLT_SHADER_COMPILER_HPP
#define GLT_SHADER_COMPILER_HPP

#include <string>
#include <map>
#include <vector>
#include <set>
#include <queue>

#include "opengl.h"

#include "data/Ref.hpp"

#include "sys/fs/fs.hpp"

#include "glt/ShaderManager.hpp"

#include "err/WithError.hpp"

namespace glt {

struct ShaderSource;
struct ShaderObject;
struct ShaderCache;
struct CompileJob;
struct CompileState;

typedef std::string ShaderSourceKey;
typedef std::queue<Ref<CompileJob> > CompileJobs;
typedef std::set<ShaderSourceKey> QueuedJobs;
typedef std::map<ShaderSourceKey, WeakRef<ShaderObject> > ShaderCacheEntries;
typedef std::map<ShaderSourceKey, Ref<ShaderObject> > ShaderObjects;
typedef std::vector<std::string> IncludePath;
typedef std::pair<std::string, sys::fs::ModificationTime> ShaderInclude;
typedef std::vector<ShaderInclude> ShaderIncludes;
typedef std::string ShaderSourceFilePath; // absolute path
typedef std::vector<Ref<ShaderSource> > ShaderDependencies;

typedef uint32 ShaderCompileFlags;

struct ShaderSource {
    ShaderSourceKey key;
    ShaderManager::ShaderType type;

    ShaderSource(const ShaderSourceKey& _key, ShaderManager::ShaderType ty) :
        key(_key), type(ty) {}
    
    virtual ~ShaderSource() {}
    
    virtual Ref<ShaderObject> load(Ref<ShaderSource>&, CompileState&) = 0;

private:
    ShaderSource(const ShaderSource&);
    ShaderSource& operator =(const ShaderSource&);
};

enum ReloadState {
    ReloadUptodate,
    ReloadOutdated,
    ReloadFailed
};

struct ShaderObject {
    Ref<ShaderSource> source;
    GLuint handle;
    ShaderIncludes includes;
    ShaderDependencies dependencies;
    WeakRef<ShaderCache> cache;
    
    ShaderObject(const Ref<ShaderSource>& src, GLuint hndl) :
        source(src), handle(hndl), includes(), dependencies(), cache() {}

    virtual ~ShaderObject();

    virtual ReloadState needsReload() = 0;

    void linkCache(Ref<ShaderCache>&);
    
    void unlinkCache(Ref<ShaderCache>&);
    
private:
    ShaderObject(const ShaderObject&);
    ShaderObject& operator =(const ShaderObject&);
};

struct ShaderCache {
    ShaderCacheEntries entries;

    ShaderCache() :
        entries() {}
    
    ~ShaderCache();

    bool lookup(Ref<ShaderObject> *, const ShaderSourceKey&);
    static bool put(Ref<ShaderCache>&, Ref<ShaderObject>&);
    static bool remove(Ref<ShaderCache>&, ShaderObject *);
    void flush();

private:
    ShaderCache(const ShaderCache&);
    ShaderCache& operator =(const ShaderCache&);
};

static const ShaderCompileFlags SC_LOOKUP_CACHE = 1 << 0;
static const ShaderCompileFlags SC_PUT_CACHE = 1 << 1;
static const ShaderCompileFlags SC_CHECK_OUTDATED = 1 << 2; // only use cache if file didnt get changed

static const ShaderCompileFlags SC_DEFAULT_FLAGS
  = SC_LOOKUP_CACHE | SC_PUT_CACHE | SC_CHECK_OUTDATED;

namespace ShaderCompilerError {

enum Type {
    NoError,
    CouldntGuessShaderType,
    InvalidShaderType,
    FileNotFound,
    FileNotInShaderPath,
    CompilationFailed,
    OpenGLError
};

std::string stringError(Type);

} // namespace ShaderCompilerError

struct ShaderCompiler {
    glt::ShaderManager& shaderManager;
    PreprocessorDefinitions defines;
    Ref<ShaderCache> cache;

    ShaderCompiler(glt::ShaderManager& mng) :
        shaderManager(mng),
        defines(),
        cache() {}

    ~ShaderCompiler();

    void init();

    static bool guessShaderType(const std::string& path, ShaderManager::ShaderType *res);
    
private:
    ShaderCompiler(const ShaderCompiler&);
    ShaderCompiler& operator =(const ShaderCompiler&);
};

struct CompileJob {
    virtual ~CompileJob() {}
    virtual Ref<ShaderSource>& source() = 0;
    virtual Ref<ShaderObject> exec(CompileState&) = 0;

    static Ref<CompileJob> load(const Ref<ShaderSource>&);
    static Ref<CompileJob> reload(const Ref<ShaderObject>&);
};

struct CompileState : public err::WithError<ShaderCompilerError::Type,
                                            ShaderCompilerError::NoError,
                                            ShaderCompilerError::stringError> {
    ShaderCompiler& compiler;
    const ShaderCompileFlags flags;
    ShaderObjects &compiled;
    QueuedJobs inQueue;
    CompileJobs toCompile;
    
    CompileState(ShaderCompiler& comp, ShaderObjects& _compiled, ShaderCompileFlags flgs = SC_DEFAULT_FLAGS) :
        compiler(comp), flags(flgs), compiled(_compiled), inQueue(), toCompile() {}

    void enqueue(Ref<CompileJob>&);

    void compileAll();

    void put(const Ref<ShaderObject>&);
    
    Ref<ShaderObject> load(Ref<ShaderSource>&);

    Ref<ShaderObject> reload(Ref<ShaderObject>&);
};

struct StringSource : public ShaderSource {
    std::string code;
    
    StringSource(ShaderManager::ShaderType ty, const std::string& _code);
    
    Ref<ShaderObject> load(Ref<ShaderSource>&, CompileState&);
};

struct FileSource : public ShaderSource {
    FileSource(ShaderManager::ShaderType ty, const std::string& path);
    
    const std::string& filePath() const { return this->key; }
    
    Ref<ShaderObject> load(Ref<ShaderSource>&, CompileState&);
};

} // namespace glt

#endif
