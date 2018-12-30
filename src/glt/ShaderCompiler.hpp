#ifndef GLT_SHADER_COMPILER_HPP
#define GLT_SHADER_COMPILER_HPP

#include <map>
#include <memory>
#include <queue>
#include <set>
#include <string>
#include <vector>

#include "err/WithError.hpp"
#include "err/err.hpp"
#include "glt/GLObject.hpp"
#include "glt/ShaderManager.hpp"
#include "glt/conf.hpp"
#include "opengl.hpp"
#include "sys/fs.hpp"

namespace glt {

struct ShaderSource;
struct ShaderObject;
struct ShaderCache;
struct CompileJob;
struct CompileState;

typedef std::string ShaderSourceKey;
typedef std::queue<std::shared_ptr<CompileJob>> CompileJobs;
typedef std::set<ShaderSourceKey> QueuedJobs;
typedef int Version;
typedef std::pair<Version, std::weak_ptr<ShaderObject>> ShaderCacheEntry;
typedef std::multimap<ShaderSourceKey, ShaderCacheEntry> ShaderCacheEntries;
typedef std::map<ShaderSourceKey, std::shared_ptr<ShaderObject>> ShaderObjects;
typedef std::vector<std::string> IncludePath;
typedef std::pair<std::string, sys::fs::FileTime> ShaderInclude;
typedef std::vector<ShaderInclude> ShaderIncludes;
typedef std::string ShaderSourceFilePath; // absolute path
typedef std::vector<std::shared_ptr<ShaderSource>> ShaderDependencies;
typedef std::map<ShaderSourceKey, std::shared_ptr<ShaderSource>>
  ShaderRootDependencies;

typedef uint32 ShaderCompileFlags;

struct GLT_API ShaderSource
{
    ShaderSourceKey key;
    ShaderManager::ShaderType type;

    ShaderSource(const ShaderSourceKey &_key, ShaderManager::ShaderType ty)
      : key(_key), type(ty)
    {}

    virtual ~ShaderSource();

    virtual std::shared_ptr<ShaderObject> load(std::shared_ptr<ShaderSource> &,
                                               CompileState &) = 0;

private:
    ShaderSource(const ShaderSource &);
    ShaderSource &operator=(const ShaderSource &);
};

enum ReloadState
{
    ReloadUptodate,
    ReloadOutdated,
    ReloadFailed
};

struct GLT_API ShaderObject
{
    std::shared_ptr<ShaderSource> source;
    GLShaderObject handle;
    ShaderIncludes includes;
    ShaderDependencies dependencies;
    std::weak_ptr<ShaderCache> cache;

    ShaderObject(const std::shared_ptr<ShaderSource> &src, GLuint hndl)
      : source(src), handle(hndl), includes(), dependencies(), cache()
    {}

    virtual ~ShaderObject();

    virtual ReloadState needsReload() = 0;
    void linkCache(std::shared_ptr<ShaderCache> &);
    void unlinkCache(std::shared_ptr<ShaderCache> &);

private:
    ShaderObject(const ShaderObject &);
    ShaderObject &operator=(const ShaderObject &);
};

struct GLT_API ShaderCache
{
    ShaderCacheEntries entries;

    ShaderCache() : entries() {}

    ~ShaderCache();

    std::shared_ptr<ShaderObject> lookup(const ShaderSourceKey &);
    static bool put(std::shared_ptr<ShaderCache> &,
                    std::shared_ptr<ShaderObject> &);
    static bool remove(std::shared_ptr<ShaderCache> &, ShaderObject *);
    void flush();
    void checkValid();

private:
    ShaderCache(const ShaderCache &);
    ShaderCache &operator=(const ShaderCache &);
};

static const ShaderCompileFlags SC_LOOKUP_CACHE = 1 << 0;
static const ShaderCompileFlags SC_PUT_CACHE = 1 << 1;
static const ShaderCompileFlags SC_CHECK_OUTDATED =
  1 << 2; // only use cache if file didnt get changed

static const ShaderCompileFlags SC_DEFAULT_FLAGS =
  SC_LOOKUP_CACHE | SC_PUT_CACHE | SC_CHECK_OUTDATED;

namespace ShaderCompilerError {

enum Type
{
    NoError,
    CouldntGuessShaderType,
    InvalidShaderType,
    FileNotFound,
    FileNotInShaderPath,
    CompilationFailed,
    OpenGLError
};

std::string GLT_API stringError(Type);

} // namespace ShaderCompilerError

struct GLT_API ShaderCompiler
{
    glt::ShaderManager &shaderManager;
    PreprocessorDefinitions defines;
    std::shared_ptr<ShaderCache> cache;

    ShaderCompiler(glt::ShaderManager &mng)
      : shaderManager(mng), defines(), cache()
    {}

    ~ShaderCompiler();

    void init();

    static bool guessShaderType(const std::string &path,
                                ShaderManager::ShaderType *res);

private:
    ShaderCompiler(const ShaderCompiler &);
    ShaderCompiler &operator=(const ShaderCompiler &);
};

struct GLT_API CompileJob
{
    virtual ~CompileJob();
    virtual std::shared_ptr<ShaderSource> &source() = 0;
    virtual std::shared_ptr<ShaderObject> exec(CompileState &) = 0;

    static std::shared_ptr<CompileJob> load(
      const std::shared_ptr<ShaderSource> &);
    static std::shared_ptr<CompileJob> reload(
      const std::shared_ptr<ShaderObject> &);
};

struct GLT_API CompileState
  : public err::WithError<ShaderCompilerError::Type,
                          ShaderCompilerError::NoError,
                          ShaderCompilerError::stringError>
{
    ShaderCompiler &compiler;
    const ShaderCompileFlags flags;
    ShaderObjects &compiled;
    QueuedJobs inQueue;
    CompileJobs toCompile;

    CompileState(ShaderCompiler &comp,
                 ShaderObjects &_compiled,
                 ShaderCompileFlags flgs = SC_DEFAULT_FLAGS)
      : compiler(comp), flags(flgs), compiled(_compiled), inQueue(), toCompile()
    {}

    void enqueue(std::shared_ptr<CompileJob> &);
    void compileAll();
    void put(const std::shared_ptr<ShaderObject> &);
    std::shared_ptr<ShaderObject> load(std::shared_ptr<ShaderSource> &);
    std::shared_ptr<ShaderObject> reload(std::shared_ptr<ShaderObject> &);
};

struct GLT_API StringSource : public ShaderSource
{
    std::string code;
    StringSource(ShaderManager::ShaderType ty, const std::string &name);
    virtual std::shared_ptr<ShaderObject> load(std::shared_ptr<ShaderSource> &,
                                               CompileState &) final override;
};

struct GLT_API FileSource : public ShaderSource
{
    FileSource(ShaderManager::ShaderType ty, const std::string &path);
    const std::string &filePath() const { return this->key; }
    virtual std::shared_ptr<ShaderObject> load(std::shared_ptr<ShaderSource> &,
                                               CompileState &) final override;
};

} // namespace glt

#endif
