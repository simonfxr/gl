#ifndef SHADER_MANAGER_HPP
#define SHADER_MANAGER_HPP

#include <ostream>
#include <string>
#include <vector>

#include "data/Ref.hpp"

namespace glt {

struct ShaderObjectCache;

struct ShaderManager {
    enum Verbosity {
        Quiet,
        OnlyErrors,
        Info
    };

    enum ShaderProfile {
        CoreProfile,
        CompatibilityProfile
    };

    enum ShaderType {
        GuessShaderType,
        VertexShader,
        FragmentShader,
        GeometryShader,
        TesselationControl,
        TesselationEvaluation
    };

    ShaderManager();
    ~ShaderManager();

    Ref<ShaderProgram> program(const std::string& name) const;
    void addProgram(const std::string& name, Ref<ShaderProgram>& program);
    Ref<ShaderProgram> declareProgram(const std::string& name);

    void reloadShaders();

    Verbosity verbosity() const;
    void verbosity(Verbosity v);

    std::ostream& err() const;
    void err(std::ostream& out);

    bool addShaderDirectory(const std::string& directory, bool check_exists = true);
    const std::vector<std::string>& shaderDirectories() const;

    void setShaderVersion(uint32 vers /* e.g. 330 */, ShaderProfile profile = CompatibilityProfile);
    uint32 shaderVersion() const;
    ShaderProfile shaderProfile() const;

    bool cacheShaderObjects() const;
    void cacheShaderObjects(bool docache);

    ShaderCompiler& shaderCompiler();
}

    
private:
    
    friend struct ShaderObjectCache;

    struct Data;
    friend struct Data;
    Data * const self;

    ShaderManager(const ShaderManager& _);
    ShaderManager& operator =(const ShaderManager& _);
};

} // namespace glt

#endif
