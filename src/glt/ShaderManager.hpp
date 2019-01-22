#ifndef SHADER_MANAGER_HPP
#define SHADER_MANAGER_HPP

#include "bl/hashtable.hpp"
#include "bl/shared_ptr.hpp"
#include "bl/string.hpp"
#include "bl/vector.hpp"
#include "glt/conf.hpp"
#include "pp/enum.hpp"
#include "pp/pimpl.hpp"
#include "sys/io/Stream.hpp"

namespace glt {

struct ShaderCompiler;
struct ShaderProgram;
struct ShaderCache;

typedef bl::hashtable<bl::string, bl::string> PreprocessorDefinitions;

typedef bl::vector<bl::string> ShaderDirectories;

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

    bl::shared_ptr<ShaderProgram> program(const bl::string &name) const;
    void addProgram(const bl::string &name,
                    bl::shared_ptr<ShaderProgram> &program);
    bl::shared_ptr<ShaderProgram> declareProgram(const bl::string &name);

    void reloadShaders();

    ShaderManagerVerbosity verbosity() const;
    void verbosity(ShaderManagerVerbosity v);

    sys::io::OutStream &out() const;
    void out(sys::io::OutStream &out);

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

    const bl::shared_ptr<ShaderCache> &globalShaderCache();

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
