#ifndef GLT_SHADER_COMPILER_HPP
#define GLT_SHADER_COMPILER_HPP

#include "glt/conf.hpp"

#include "err/WithError.hpp"
#include "err/err.hpp"
#include "glt/GLObject.hpp"
#include "glt/ShaderManager.hpp"
#include "opengl.hpp"
#include "sys/fs.hpp"

#include <memory>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace glt {

struct ShaderSource;
struct ShaderObject;
struct ShaderCache;
struct ShaderCompiler;
struct ShaderCompilerQueue;

using ShaderSourceKey = std::string;

using ShaderObjects =
  std::unordered_map<ShaderSourceKey, std::shared_ptr<ShaderObject>>;

using ShaderRootDependencies =
  std::unordered_map<ShaderSourceKey, std::shared_ptr<ShaderSource>>;

using IncludePath = std::vector<std::string>;

using ShaderInclude = std::pair<std::string, sys::fs::FileTime>;

using ShaderIncludes = std::vector<ShaderInclude>;

using ShaderDependencies = std::vector<std::shared_ptr<ShaderSource>>;

using ShaderCompileFlags = uint32;

using Version = int;

using ShaderCacheEntry = std::pair<Version, std::weak_ptr<ShaderObject>>;

using ShaderCacheEntries =
  std::unordered_multimap<ShaderSourceKey, ShaderCacheEntry>;

static const ShaderCompileFlags SC_LOOKUP_CACHE = 1 << 0;
static const ShaderCompileFlags SC_PUT_CACHE = 1 << 1;
static const ShaderCompileFlags SC_CHECK_OUTDATED =
  1 << 2; // only use cache if file didnt get changed

static const ShaderCompileFlags SC_DEFAULT_FLAGS =
  SC_LOOKUP_CACHE | SC_PUT_CACHE | SC_CHECK_OUTDATED;

enum ReloadState : uint8
{
    Failed,
    Uptodate,
    Outdated
};

struct GLT_API ShaderSource
{
    static std::shared_ptr<ShaderSource> makeStringSource(
      ShaderManager::ShaderType ty,
      const std::string &source);

    static std::shared_ptr<ShaderSource> makeFileSource(
      ShaderManager::ShaderType ty,
      const std::string &path);

    const ShaderSourceKey &key() const;

private:
    friend struct ShaderCompiler;
    friend struct ShaderCompilerQueue;
    friend struct ShaderCache;
    friend struct ShaderObject;

    struct Data;
    struct DataDeleter
    {
        void operator()(Data *) noexcept;
    };
    const std::unique_ptr<Data, DataDeleter> self;

public:
    explicit ShaderSource(Data &&);
};

struct GLT_API ShaderObject
{
    const GLShaderObject &handle() const;
    const std::shared_ptr<ShaderSource> &shaderSource();

private:
    friend struct ShaderCompiler;
    friend struct ShaderSource;
    friend struct ShaderCompilerQueue;
    friend struct ShaderCache;

    struct Data;
    struct DataDeleter
    {
        void operator()(Data *) noexcept;
    };
    const std::unique_ptr<Data, DataDeleter> self;

public:
    ShaderObject(Data *);
};

struct GLT_API ShaderCache
{
    void flush();

    static bool remove(
      const std::shared_ptr<ShaderCache> & /* pointer to this */,
      ShaderObject *);

    static bool put(const std::shared_ptr<ShaderCache> & /* pointer to this */,
                    const std::shared_ptr<ShaderObject> &);

    std::shared_ptr<ShaderObject> lookup(const ShaderSourceKey &);

    const ShaderCacheEntries &cacheEntries() const;

private:
    struct Data;
    struct DataDeleter
    {
        void operator()(Data *) noexcept;
    };
    const std::unique_ptr<Data, DataDeleter> self;
};

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
    ShaderCompiler(ShaderManager &);

    ShaderManager &shaderManager();
    const std::shared_ptr<ShaderCache> &shaderCache();

    void init();

    static bool guessShaderType(const std::string &path,
                                ShaderManager::ShaderType *res);

private:
    friend struct ShaderSource;
    struct Data;
    struct DataDeleter
    {
        void operator()(Data *) noexcept;
    };
    const std::unique_ptr<Data, DataDeleter> self;
};

struct GLT_API ShaderCompilerQueue
  : public err::WithError<ShaderCompilerError::Type,
                          ShaderCompilerError::NoError,
                          ShaderCompilerError::stringError>
{
    ShaderCompilerQueue(ShaderCompiler &comp,
                        ShaderObjects &_compiled,
                        ShaderCompileFlags flgs = SC_DEFAULT_FLAGS);

    ShaderCompiler &shaderCompiler();

    void enqueueLoad(const std::shared_ptr<ShaderSource> &);
    void enqueueReload(const std::shared_ptr<ShaderObject> &);
    void compileAll();

private:
    struct Data;
    struct DataDeleter
    {
        void operator()(Data *) noexcept;
    };
    const std::unique_ptr<Data, DataDeleter> self;
};

} // namespace glt

#endif
