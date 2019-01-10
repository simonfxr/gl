#ifndef SHADER_MANAGER_HPP
#define SHADER_MANAGER_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "glt/conf.hpp"
#include "sys/io/Stream.hpp"

namespace glt {

struct ShaderCompiler;
struct ShaderProgram;
struct ShaderCache;

typedef std::unordered_map<std::string, std::string> PreprocessorDefinitions;

typedef std::vector<std::string> ShaderDirectories;

struct GLT_API ShaderManager
{

    enum Verbosity
    {
        Quiet,
        OnlyErrors,
        Info
    };

    enum ShaderProfile
    {
        CoreProfile,
        CompatibilityProfile
    };

    enum ShaderType
    {
        GuessShaderType,
        VertexShader,
        FragmentShader,
        GeometryShader,
        TesselationControl,
        TesselationEvaluation
    };

    ShaderManager();

    std::shared_ptr<ShaderProgram> program(const std::string &name) const;
    void addProgram(const std::string &name,
                    std::shared_ptr<ShaderProgram> &program);
    std::shared_ptr<ShaderProgram> declareProgram(const std::string &name);

    void reloadShaders();

    Verbosity verbosity() const;
    void verbosity(Verbosity v);

    sys::io::OutStream &out() const;
    void out(sys::io::OutStream &out);

    bool prependShaderDirectory(const std::string &directory,
                                bool check_exists = true);
    bool addShaderDirectory(const std::string &directory,
                            bool check_exists = true);
    bool removeShaderDirectory(const std::string &dir);
    const ShaderDirectories &shaderDirectories() const;

    void setShaderVersion(uint32_t vers /* e.g. 330 */,
                          ShaderProfile profile = CompatibilityProfile);
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

private:
    DECLARE_PIMPL(GLT_API, self);
};

} // namespace glt

#endif
