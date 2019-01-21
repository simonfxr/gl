#ifndef SHADER_MANAGER_HPP
#define SHADER_MANAGER_HPP

#include "glt/conf.hpp"
#include "pp/enum.hpp"
#include "pp/pimpl.hpp"
#include "sys/io/Stream.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace glt {

struct ShaderCompiler;
struct ShaderProgram;
struct ShaderCache;

typedef std::unordered_map<std::string, std::string> PreprocessorDefinitions;

typedef std::vector<std::string> ShaderDirectories;

#define GLT_SHADER_MANAGER_VERBOSITY_ENUM_DEF(T, V0, V)                        \
    T(ShaderManagerVerbosity, uint8_t, V0(Quiet) V(OnlyErrors) V(Info))

#define GLT_SHADER_PROFILE_ENUM_DEF(T, V0, V)                                  \
    T(ShaderProfile, uint8_t, V0(Core) V(Compatibility))

#define GLT_SHADER_TYPE_ENUM_DEF(T, V0, V)                                     \
    T(ShaderType,                                                              \
      uint8_t,                                                                 \
      V0(GuessShaderType) V(VertexShader) V(FragmentShader) V(GeometryShader)  \
        V(TesselationControl) V(TesselationEvaluation))

PP_DEF_ENUM_WITH_API(GLT_API, GLT_SHADER_MANAGER_VERBOSITY_ENUM_DEF);
PP_DEF_ENUM_WITH_API(GLT_API, GLT_SHADER_PROFILE_ENUM_DEF);
PP_DEF_ENUM_WITH_API(GLT_API, GLT_SHADER_TYPE_ENUM_DEF);

struct GLT_API ShaderManager
{
    ShaderManager();
    ~ShaderManager();

    std::shared_ptr<ShaderProgram> program(const std::string &name) const;
    void addProgram(const std::string &name,
                    std::shared_ptr<ShaderProgram> &program);
    std::shared_ptr<ShaderProgram> declareProgram(const std::string &name);

    void reloadShaders();

    ShaderManagerVerbosity verbosity() const;
    void verbosity(ShaderManagerVerbosity v);

    sys::io::OutStream &out() const;
    void out(sys::io::OutStream &out);

    bool prependShaderDirectory(const std::string &directory,
                                bool check_exists = true);
    bool addShaderDirectory(const std::string &directory,
                            bool check_exists = true);
    bool removeShaderDirectory(const std::string &dir);
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

private:
    struct Data;
    Data *const self;
};

} // namespace glt

#endif
