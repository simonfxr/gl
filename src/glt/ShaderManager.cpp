#include "glt/ShaderManager.hpp"

#include "glt/ShaderCompiler.hpp"
#include "glt/ShaderProgram.hpp"
#include "util/string.hpp"

#include <bl/box.hpp>

#include <algorithm>
#include <unordered_map>

namespace glt {

using namespace bl::literals;

using ProgramMap =
  std::unordered_map<bl::string, std::shared_ptr<ShaderProgram>>;

struct ShaderManager::Impl : public ShaderManager
{
    sys::io::OutStream out;
    ShaderDirectories shaderDirs;
    ProgramMap programs;
    std::shared_ptr<ShaderCache> globalShaderCache;
    PreprocessorDefinitions globalDefines;
    bl::box<ShaderCompiler> shaderCompiler;
    uint32_t shader_version{};
    ShaderProfile shader_profile{ ShaderProfile::Core };
    ShaderManagerVerbosity verbosity{ ShaderManagerVerbosity::Info };
    bool cache_so{ true };
    bool dumpShaders{ false };

    explicit Impl()
      : out(sys::io::stdout())
      , globalShaderCache(std::make_shared<ShaderCache>())
      , shaderCompiler(bl::in_place, *this)
    {}
};

BL_DEF_RUNTIME_SIZED(ShaderManager, ShaderManager::Impl);

ShaderManager &
ShaderManager::construct_at(void *p) noexcept
{
    return construct_impl(p);
}

void
ShaderManager::shutdown()
{
    m().programs.clear();
    m().globalShaderCache->flush();
}

void
ShaderManager::setShaderVersion(uint32_t vers, ShaderProfile prof)
{
    ShaderProfile newprof = ShaderProfile::Core;
    if (prof == ShaderProfile::Compatibility)
        newprof = prof;
    m().shader_version = vers;
    m().shader_profile = newprof;
}

uint32_t
ShaderManager::shaderVersion() const
{
    return m().shader_version;
}

ShaderProfile
ShaderManager::shaderProfile() const
{
    return m().shader_profile;
}

ShaderManagerVerbosity
ShaderManager::verbosity() const
{
    return m().verbosity;
}

void
ShaderManager::verbosity(ShaderManagerVerbosity v)
{
    m().verbosity = v;
}

std::shared_ptr<ShaderProgram>
ShaderManager::program(const bl::string &name) const
{
    auto it = m().programs.find(name);
    if (it != m().programs.end())
        return it->second;
    ERR(scat("program not found: "_sv, name));
    return {};
}

void
ShaderManager::addProgram(const bl::string &name,
                          std::shared_ptr<ShaderProgram> &program)
{
    ASSERT(program);
    m().programs[name] = program;
}

std::shared_ptr<ShaderProgram>
ShaderManager::declareProgram(const bl::string &name)
{
    auto prog = std::make_shared<ShaderProgram>(*this);
    m().programs[name] = prog;
    return prog;
}

void
ShaderManager::reloadShaders()
{
    const auto n = m().programs.size();
    auto failed = size_t{ 0 };
    for (auto &ent : m().programs)
        if (!ent.second->reload())
            ++failed;

    out() << "all shaders reloaded ("_k << (n - failed) << " successful, "_k
          << failed << " failed)\n"_k;
}

sys::io::OutStream
ShaderManager::out() const
{
    return m().out;
}

void
ShaderManager::out(sys::io::OutStream out)
{
    m().out = out;
}

bool
ShaderManager::prependShaderDirectory(const bl::string &dir, bool check_exists)
{
    removeShaderDirectory(dir);

    if (check_exists && !sys::fs::directoryExists(dir))
        return false;

    m().shaderDirs.insert(m().shaderDirs.begin(), dir);
    return true;
}

bool
ShaderManager::addShaderDirectory(const bl::string &dir, bool check_exists)
{
    removeShaderDirectory(dir);

    if (check_exists && !sys::fs::directoryExists(dir))
        return false;

    m().shaderDirs.push_back(dir);
    return true;
}

bool
ShaderManager::removeShaderDirectory(const bl::string &dir)
{
    auto &dirs = m().shaderDirs;
    auto it = find(dirs.begin(), dirs.end(), dir);
    if (it == dirs.end())
        return false;
    dirs.erase(it);
    return true;
}

const ShaderDirectories &
ShaderManager::shaderDirectories() const
{
    return m().shaderDirs;
}

bool
ShaderManager::cacheShaderObjects() const
{
    return m().cache_so;
}

void
ShaderManager::cacheShaderObjects(bool docache)
{
    if (docache != m().cache_so) {
        m().cache_so = docache;
        if (!docache)
            m().globalShaderCache->flush();
    }
}

bool
ShaderManager::dumpShadersEnabled() const
{
    return m().dumpShaders;
}

void
ShaderManager::dumpShadersEnable(bool enable)
{
    m().dumpShaders = enable;
}

const std::shared_ptr<ShaderCache> &
ShaderManager::globalShaderCache()
{
    return m().globalShaderCache;
}

ShaderCompiler &
ShaderManager::shaderCompiler()
{
    return *m().shaderCompiler;
}

PreprocessorDefinitions &
ShaderManager::globalDefines()
{
    return m().globalDefines;
}

const PreprocessorDefinitions &
ShaderManager::globalDefines() const
{
    return m().globalDefines;
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
