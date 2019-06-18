#include "glt/ShaderManager.hpp"

#include "glt/ShaderCompiler.hpp"
#include "glt/ShaderProgram.hpp"

#include "bl/algorithm.hpp"

#include <unordered_map>

namespace glt {

USE_STRING_LITERALS;

PP_DEF_ENUM_IMPL(GLT_SHADER_MANAGER_VERBOSITY_ENUM_DEF);
PP_DEF_ENUM_IMPL(GLT_SHADER_PROFILE_ENUM_DEF);
PP_DEF_ENUM_IMPL(GLT_SHADER_TYPE_ENUM_DEF);

using ProgramMap = bl::hashtable<bl::string, bl::shared_ptr<ShaderProgram>>;

struct ShaderManager::Data
{
    ShaderManagerVerbosity verbosity;
    sys::io::OutStream *out;
    bl::vector<bl::string> shaderDirs;
    ProgramMap programs;
    uint32_t shader_version;
    ShaderProfile shader_profile;
    bool cache_so;
    bool dumpShaders{ false };
    bl::shared_ptr<ShaderCache> globalShaderCache;
    PreprocessorDefinitions globalDefines;
    ShaderCompiler shaderCompiler;
    ShaderManager &self;

    explicit Data(ShaderManager &me)
      : verbosity(ShaderManagerVerbosity::Info)
      , out(&sys::io::stdout())
      , shader_version(0)
      , shader_profile(ShaderProfile::Core)
      , cache_so(true)
      , globalShaderCache(bl::make_shared<ShaderCache>())
      , shaderCompiler(me)
      , self(me)
    {}

private:
    Data(const Data &) = delete;
    Data &operator=(const Data &) = delete;
};

ShaderManager::ShaderManager() : self(new Data(*this))
{
    self->shaderCompiler.init();
}

ShaderManager::~ShaderManager()
{
    shutdown();
    delete self;
}

void
ShaderManager::shutdown()
{
    self->programs.clear();
    self->globalShaderCache->flush();
}

void
ShaderManager::setShaderVersion(uint32_t vers, ShaderProfile prof)
{
    ShaderProfile newprof = ShaderProfile::Core;
    if (prof == ShaderProfile::Compatibility)
        newprof = prof;
    self->shader_version = vers;
    self->shader_profile = newprof;
}

uint32_t
ShaderManager::shaderVersion() const
{
    return self->shader_version;
}

ShaderProfile
ShaderManager::shaderProfile() const
{
    return self->shader_profile;
}

ShaderManagerVerbosity
ShaderManager::verbosity() const
{
    return self->verbosity;
}

void
ShaderManager::verbosity(ShaderManagerVerbosity v)
{
    self->verbosity = v;
}

bl::shared_ptr<ShaderProgram>
ShaderManager::program(const bl::string &name) const
{
    auto it = self->programs.find(name);
    if (it != self->programs.end())
        return it->value;
    ERR("program not found: "_sv + name);
    return {};
}

void
ShaderManager::addProgram(const bl::string &name,
                          bl::shared_ptr<ShaderProgram> &program)
{
    ASSERT(program);
    self->programs[name] = program;
}

bl::shared_ptr<ShaderProgram>
ShaderManager::declareProgram(const bl::string &name)
{
    auto prog = bl::make_shared<ShaderProgram>(*this);
    self->programs[name] = prog;
    return prog;
}

void
ShaderManager::reloadShaders()
{
    const auto n = self->programs.size();
    auto failed = size_t{ 0 };
    for (auto &ent : self->programs)
        if (!ent.value->reload())
            ++failed;

    out() << "all shaders reloaded (" << (n - failed) << " successful, "
          << failed << " failed)" << sys::io::endl;
}

sys::io::OutStream &
ShaderManager::out() const
{
    return *self->out;
}

void
ShaderManager::out(sys::io::OutStream &out)
{
    self->out = &out;
}

bool
ShaderManager::prependShaderDirectory(const bl::string &dir, bool check_exists)
{
    removeShaderDirectory(dir);

    if (check_exists && !sys::fs::directoryExists(dir))
        return false;

    self->shaderDirs.push_front(dir);
    return true;
}

bool
ShaderManager::addShaderDirectory(const bl::string &dir, bool check_exists)
{
    removeShaderDirectory(dir);

    if (check_exists && !sys::fs::directoryExists(dir))
        return false;

    self->shaderDirs.push_back(dir);
    return true;
}

bool
ShaderManager::removeShaderDirectory(const bl::string &dir)
{
    ShaderDirectories &dirs = self->shaderDirs;
    auto it = bl::find(dirs.begin(), dirs.end(), dir);
    if (it == dirs.end())
        return false;
    dirs.erase(it);
    return true;
}

const bl::vector<bl::string> &
ShaderManager::shaderDirectories() const
{
    return self->shaderDirs;
}

bool
ShaderManager::cacheShaderObjects() const
{
    return self->cache_so;
}

void
ShaderManager::cacheShaderObjects(bool docache)
{
    if (docache != self->cache_so) {
        self->cache_so = docache;
        if (!docache)
            self->globalShaderCache->flush();
    }
}

bool
ShaderManager::dumpShadersEnabled() const
{
    return self->dumpShaders;
}

void
ShaderManager::dumpShadersEnable(bool enable)
{
    self->dumpShaders = enable;
}

const bl::shared_ptr<ShaderCache> &
ShaderManager::globalShaderCache()
{
    return self->globalShaderCache;
}

ShaderCompiler &
ShaderManager::shaderCompiler()
{
    return self->shaderCompiler;
}

PreprocessorDefinitions &
ShaderManager::globalDefines()
{
    return self->globalDefines;
}

const PreprocessorDefinitions &
ShaderManager::globalDefines() const
{
    return self->globalDefines;
}

uint32_t
ShaderManager::glToShaderVersion(uint32_t maj, uint32_t min)
{
    if (maj > 3 || (maj == 3 && min >= 3))
        return maj * 100 + min * 10;
    if (maj == 3)
        return 130 + min * 10;
    if (maj == 2)
        return 110 + min * 10;
    return 0;
}

} // namespace glt
