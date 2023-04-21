#ifndef GLT_SHADER_COMPILER_HPP
#define GLT_SHADER_COMPILER_HPP

#include "glt/conf.hpp"

#include "err/WithError.hpp"
#include "err/err.hpp"
#include "glt/GLObject.hpp"
#include "glt/ShaderManager.hpp"
#include "opengl.hpp"
#include "pp/enum.hpp"
#include "pp/pimpl.hpp"
#include "sys/fs.hpp"

#include <bl/new.hpp>

#include <bl/string.hpp>
#include <bl/string_view.hpp>
#include <memory>
#include <unordered_map>
#include <vector>

namespace glt {

struct ShaderSource;
struct ShaderObject;
struct ShaderCache;
struct ShaderCompiler;
struct ShaderCompilerQueue;

using ShaderSourceKey = bl::string;

using ShaderObjects =
  std::unordered_map<ShaderSourceKey, std::shared_ptr<ShaderObject>>;

using ShaderRootDependencies =
  std::unordered_map<ShaderSourceKey, std::shared_ptr<ShaderSource>>;

using IncludePath = std::vector<bl::string>;

using ShaderInclude = std::pair<bl::string, sys::fs::FileTime>;

using ShaderIncludes = std::vector<ShaderInclude>;

using ShaderDependencies = std::vector<std::shared_ptr<ShaderSource>>;

using ShaderCompileFlags = uint32_t;

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

enum ReloadState : uint8_t
{
    Failed,
    Uptodate,
    Outdated
};

struct GLT_API ShaderSource : std::enable_shared_from_this<ShaderSource>
{
    static std::shared_ptr<ShaderSource> makeStringSource(ShaderType ty,
                                                          bl::string source);

    static std::shared_ptr<ShaderSource> makeFileSource(ShaderType ty,
                                                        bl::string path);

    const ShaderSourceKey &key() const;

private:
    friend struct ShaderCompiler;
    friend struct ShaderCompilerQueue;
    friend struct ShaderCache;
    friend struct ShaderObject;

    DECLARE_PIMPL(GLT_API, self);

public:
    explicit ShaderSource(Data &&);
};

struct GLT_API ShaderObject : std::enable_shared_from_this<ShaderObject>
{
    const GLShaderObject &handle() const;
    const std::shared_ptr<ShaderSource> &shaderSource();
    ~ShaderObject();

private:
    friend struct ShaderCompiler;
    friend struct ShaderSource;
    friend struct ShaderCompilerQueue;
    friend struct ShaderCache;

    DECLARE_PIMPL(GLT_API, self);
    struct InitArgs;

public:
    explicit ShaderObject(InitArgs &&);
};

struct GLT_API ShaderCache : std::enable_shared_from_this<ShaderCache>
{
    ShaderCache();
    ~ShaderCache();

    void flush();

    bool remove(ShaderObject *);
    bool put(const std::shared_ptr<ShaderObject> &);
    std::shared_ptr<ShaderObject> lookup(const ShaderSourceKey &);
    const ShaderCacheEntries &cacheEntries() const;

private:
    DECLARE_PIMPL(GLT_API, self);
};

PP_DEF_ENUM(ShaderCompilerError,
            uint8_t,
            NoError,
            CouldntGuessShaderType,
            InvalidShaderType,
            FileNotFound,
            FileNotInShaderPath,
            CompilationFailed,
            OpenGLError);

struct GLT_API ShaderCompiler : private bl::runtime_sized_base<ShaderCompiler>
{
    static ShaderCompiler &construct_at(void *, ShaderManager &) noexcept;

    ShaderManager &shaderManager();
    const std::shared_ptr<ShaderCache> &shaderCache();

    void init();

    static std::optional<ShaderType> guessShaderType(bl::string_view path);

private:
    friend struct ShaderSource;

    BL_DECLARE_RUNTIME_SIZED(ShaderCompiler, Impl);
};

struct GLT_API ShaderCompilerQueue
  : private bl::runtime_sized_base<ShaderCompilerQueue>
  , public err::WithError<ShaderCompilerError>
{
    static ShaderCompilerQueue &construct_at(
      void *,
      ShaderCompiler &comp,
      ShaderObjects &_compiled,
      ShaderCompileFlags flgs = SC_DEFAULT_FLAGS) noexcept;

    ShaderCompiler &shaderCompiler();

    void enqueueLoad(const std::shared_ptr<ShaderSource> &);
    void enqueueReload(const std::shared_ptr<ShaderObject> &);
    void compileAll();

    BL_DECLARE_RUNTIME_SIZED(ShaderCompilerQueue, Impl);
};

} // namespace glt

#endif
