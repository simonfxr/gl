#include "glt/ShaderManager.hpp"

#include "glt/ShaderCompiler.hpp"
#include "glt/ShaderProgram.hpp"

#include <algorithm>
#include <map>

namespace glt {

#define NULL_PROGRAM_REF std::shared_ptr<ShaderProgram>()

typedef std::map<std::string, std::shared_ptr<ShaderProgram>> ProgramMap;

struct ShaderManager::Data
{
    Verbosity verbosity;
    sys::io::OutStream *out;
    std::vector<std::string> shaderDirs;
    ProgramMap programs;
    uint32_t shader_version;
    ShaderProfile shader_profile;
    bool cache_so;
    bool dumpShaders{ false };
    std::shared_ptr<ShaderCache> globalShaderCache;
    PreprocessorDefinitions globalDefines;
    ShaderCompiler shaderCompiler;
    ShaderManager &self;

    explicit Data(ShaderManager &me)
      : verbosity(Info)
      , out(&sys::io::stdout())
      , shader_version(0)
      , shader_profile(CoreProfile)
      , cache_so(true)
      , globalShaderCache(new ShaderCache)
      , shaderCompiler(me)
      , self(me)
    {}

    ~Data() { self.shutdown(); }

private:
    Data(const Data &) = delete;
    Data &operator=(const Data &) = delete;
};

DECLARE_PIMPL_DEL(ShaderManager)

ShaderManager::ShaderManager() : self(new Data(*this))
{
    self->shaderCompiler.init();
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
    ShaderProfile newprof = CoreProfile;
    if (prof == CompatibilityProfile)
        newprof = CompatibilityProfile;
    self->shader_version = vers;
    self->shader_profile = newprof;
}

uint32_t
ShaderManager::shaderVersion() const
{
    return self->shader_version;
}

ShaderManager::ShaderProfile
ShaderManager::shaderProfile() const
{
    return self->shader_profile;
}

ShaderManager::Verbosity
ShaderManager::verbosity() const
{
    return self->verbosity;
}

void
ShaderManager::verbosity(ShaderManager::Verbosity v)
{
    self->verbosity = v;
}

std::shared_ptr<ShaderProgram>
ShaderManager::program(const std::string &name) const
{
    auto it = self->programs.find(name);
    if (it != self->programs.end())
        return it->second;
    ERR(("program not found: " + name).c_str());
    return NULL_PROGRAM_REF;
}

void
ShaderManager::addProgram(const std::string &name,
                          std::shared_ptr<ShaderProgram> &program)
{
    ASSERT(program);
    self->programs[name] = program;
}

std::shared_ptr<ShaderProgram>
ShaderManager::declareProgram(const std::string &name)
{
    std::shared_ptr<ShaderProgram> prog(new ShaderProgram(*this));
    self->programs[name] = prog;
    return prog;
}

void
ShaderManager::reloadShaders()
{
    const auto n = self->programs.size();
    auto failed = size_t{ 0 };
    for (auto &ent : self->programs)
        if (!ent.second->reload())
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
ShaderManager::prependShaderDirectory(const std::string &dir, bool check_exists)
{
    removeShaderDirectory(dir);

    if (check_exists) {
        auto type = sys::fs::exists(dir);
        if (!type || *type != sys::fs::Directory)
            return false;
    }

    self->shaderDirs.insert(self->shaderDirs.begin(), dir);
    return true;
}

bool
ShaderManager::addShaderDirectory(const std::string &dir, bool check_exists)
{
    removeShaderDirectory(dir);

    if (check_exists) {
        auto type = sys::fs::exists(dir);
        if (!type || *type != sys::fs::Directory)
            return false;
    }

    self->shaderDirs.push_back(dir);
    return true;
}

bool
ShaderManager::removeShaderDirectory(const std::string &dir)
{
    ShaderDirectories &dirs = self->shaderDirs;
    auto it = find(dirs.begin(), dirs.end(), dir);
    if (it == dirs.end())
        return false;
    dirs.erase(it);
    return true;
}

const std::vector<std::string> &
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

const std::shared_ptr<ShaderCache> &
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
        return maj * 100 + min;
    if (maj == 3)
        return 130 + min * 10;
    if (maj == 2)
        return 110 + min * 10;
    return 0;
}

} // namespace glt
