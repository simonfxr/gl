#ifndef GLT_SHADER_COMPILER_HPP
#define GLT_SHADER_COMPILER_HPP

#include "bl/hashtable.hpp"
#include "bl/string.hpp"
#include "bl/string_view.hpp"
#include "bl/vector.hpp"
#include "err/WithError.hpp"
#include "err/err.hpp"
#include "glt/GLObject.hpp"
#include "glt/ShaderManager.hpp"
#include "opengl.hpp"
#include "pp/enum.hpp"
#include "sys/fs.hpp"

namespace glt {

struct ShaderSource;
struct ShaderObject;
struct ShaderCache;
struct ShaderCompiler;
struct ShaderCompilerQueue;

using ShaderSourceKey = bl::string;

using ShaderObjects =
  bl::hashtable<ShaderSourceKey, bl::shared_ptr<ShaderObject>>;

using ShaderRootDependencies =
  bl::hashtable<ShaderSourceKey, bl::shared_ptr<ShaderSource>>;

using IncludePath = bl::vector<bl::string>;

struct ShaderInclude
{
    bl::string path;
    sys::fs::FileTime mtime;
};

using ShaderIncludes = bl::vector<ShaderInclude>;

using ShaderDependencies = bl::vector<bl::shared_ptr<ShaderSource>>;

using ShaderCompileFlags = uint32_t;

using Version = int;

struct ShaderCacheEntry
{
    bl::weak_ptr<ShaderObject> shaderObject;
    Version version;
};

using ShaderCacheEntries =
  bl::hashtable<ShaderSourceKey, bl::vector<ShaderCacheEntry>>;

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

struct GLT_API ShaderSource : bl::enable_shared_from_this<ShaderSource>
{
    static bl::shared_ptr<ShaderSource> makeStringSource(ShaderType ty,
                                                         bl::string source);

    static bl::shared_ptr<ShaderSource> makeFileSource(ShaderType ty,
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

struct GLT_API ShaderObject : bl::enable_shared_from_this<ShaderObject>
{
    const GLShaderObject &handle() const;
    const bl::shared_ptr<ShaderSource> &shaderSource();
    ~ShaderObject();

private:
    friend struct ShaderCompiler;
    friend struct ShaderSource;
    friend struct ShaderCompilerQueue;
    friend struct ShaderCache;

    DECLARE_PIMPL(GLT_API, self);
    struct InitArgs;

public:
    ShaderObject(InitArgs &&);
};

struct GLT_API ShaderCache : bl::enable_shared_from_this<ShaderCache>
{
    ShaderCache();
    ~ShaderCache();

    void flush();

    bool remove(ShaderObject *);
    bool put(const bl::shared_ptr<ShaderObject> &);
    bl::shared_ptr<ShaderObject> lookup(const ShaderSourceKey &);
    const ShaderCacheEntries &cacheEntries() const;

private:
    DECLARE_PIMPL(GLT_API, self);
};

// clang-format off
#define GLT_SHADER_COMPILER_ERROR_ENUM_DEF(T, V0, V)                           \
    T(ShaderCompilerError,                                                     \
      uint8_t,                                                                 \
      V0(NoError)                                                              \
      V(CouldntGuessShaderType)                                                \
      V(InvalidShaderType)                                                     \
      V(FileNotFound)                                                          \
      V(FileNotInShaderPath)                                                   \
      V(CompilationFailed)                                                     \
      V(OpenGLError))
// clang-format on
PP_DEF_ENUM_WITH_API(GLT_API, GLT_SHADER_COMPILER_ERROR_ENUM_DEF);

struct GLT_API ShaderCompiler
{
    ShaderCompiler(ShaderManager &);

    ShaderManager &shaderManager();
    const bl::shared_ptr<ShaderCache> &shaderCache();

    void init();

    static bool guessShaderType(bl::string_view path, ShaderType *res);

private:
    friend struct ShaderSource;
    DECLARE_PIMPL(GLT_API, self);
};

struct GLT_API ShaderCompilerQueue : public err::WithError<ShaderCompilerError>
{
    ShaderCompilerQueue(ShaderCompiler &comp,
                        ShaderObjects &_compiled,
                        ShaderCompileFlags flgs = SC_DEFAULT_FLAGS);

    ShaderCompiler &shaderCompiler();

    void enqueueLoad(const bl::shared_ptr<ShaderSource> &);
    void enqueueReload(const bl::shared_ptr<ShaderObject> &);
    void compileAll();

private:
    DECLARE_PIMPL(GLT_API, self);
};

} // namespace glt

#endif
