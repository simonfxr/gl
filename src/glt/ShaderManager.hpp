#ifndef SHADER_MANAGER_HPP
#define SHADER_MANAGER_HPP

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "glt/conf.hpp"
#include "sys/io/Stream.hpp"

namespace glt {

using namespace defs;

struct ShaderCompiler;
struct ShaderProgram;
struct ShaderCache;

typedef std::map<std::string, std::string> PreprocessorDefinitions;

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

    void setShaderVersion(uint32 vers /* e.g. 330 */,
                          ShaderProfile profile = CompatibilityProfile);
    uint32 shaderVersion() const;
    ShaderProfile shaderProfile() const;

    bool cacheShaderObjects() const;
    void cacheShaderObjects(bool);

    const std::shared_ptr<ShaderCache> &globalShaderCache();

    ShaderCompiler &shaderCompiler();

    PreprocessorDefinitions &globalDefines();

    const PreprocessorDefinitions &globalDefines() const;

    void shutdown();

private:
    struct Data;
    friend struct Data;

    struct DataDeleter
    {
        void operator()(Data *) noexcept;
    };

    std::unique_ptr<Data, DataDeleter> self;

    ShaderManager(const ShaderManager &_);
    ShaderManager &operator=(const ShaderManager &_);
};

} // namespace glt

#endif
