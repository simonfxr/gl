#ifndef SHADER_MANAGER_HPP
#define SHADER_MANAGER_HPP

#include "glt/conf.hpp"
#include "pp/enum.hpp"
#include "sys/io/Stream.hpp"

#include <bl/new.hpp>
#include <bl/vec.hpp>

#include <bl/string.hpp>
#include <memory>
#include <unordered_map>

namespace glt {

struct ShaderCompiler;
struct ShaderProgram;
struct ShaderCache;

using PreprocessorDefinitions = std::unordered_map<bl::string, bl::string>;

using ShaderDirectories = bl::vec<bl::string>;

PP_DEF_ENUM(ShaderManagerVerbosity, uint8_t, Quiet, OnlyErrors, Info);

PP_DEF_ENUM(ShaderProfile, uint8_t, Core, Compatibility);

PP_DEF_ENUM(ShaderType,
            uint8_t,
            GuessShaderType,
            VertexShader,
            FragmentShader,
            GeometryShader);
// ,TesselationControl
// ,TesselationEvaluation

struct GLT_API ShaderManager : private bl::runtime_sized_base<ShaderManager>
{
    static ShaderManager &construct_at(void *) noexcept;

    std::shared_ptr<ShaderProgram> program(const bl::string &name) const;
    void addProgram(const bl::string &name,
                    std::shared_ptr<ShaderProgram> &program);
    std::shared_ptr<ShaderProgram> declareProgram(const bl::string &name);

    void reloadShaders();

    ShaderManagerVerbosity verbosity() const;
    void verbosity(ShaderManagerVerbosity v);

    sys::io::OutStream out() const;
    void out(sys::io::OutStream out);

    bool prependShaderDirectory(const bl::string &directory,
                                bool check_exists = true);
    bool addShaderDirectory(const bl::string &directory,
                            bool check_exists = true);
    bool removeShaderDirectory(const bl::string &dir);
    const ShaderDirectories &shaderDirectories() const;

    void setShaderVersion(uint32_t vers /* e.g. 330 */,
                          ShaderProfile profile = ShaderProfile::Compatibility);
    uint32_t shaderVersion() const;
    ShaderProfile shaderProfile() const;

    bool cacheShaderObjects() const;
    void cacheShaderObjects(bool);

    bool dumpShadersEnabled() const;
    void dumpShadersEnable(bool);

    const std::shared_ptr<ShaderCache> &globalShaderCache();

    ShaderCompiler &shaderCompiler();

    PreprocessorDefinitions &globalDefines();

    const PreprocessorDefinitions &globalDefines() const;

    void shutdown();

    static uint32_t glToShaderVersion(uint32_t maj, uint32_t min);

    BL_DECLARE_RUNTIME_SIZED(ShaderManager, Impl);
};

} // namespace glt

#endif
