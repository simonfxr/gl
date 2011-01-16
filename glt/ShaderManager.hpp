#ifndef SHADER_MANAGER_HPP
#define SHADER_MANAGER_HPP

#include <ostream>
#include <string>
#include <vector>

namespace glt {

struct ShaderManager {

    enum Verbosity {
        Quiet,
        OnlyErrors,
        Info
    };
    
    ShaderManager();
    ~ShaderManager();

    Verbosity verbosity();
    void verbosity(Verbosity v);

    std::ostream& err();
    void err(std::ostream& out);

    void addIncludeDir(const std::string& path);
    const std::vector<std::string>& includeDirs();

private:
    
    struct Data;
    friend struct Data;
    Data * const self;

    ShaderManager(const ShaderManager& _);
    ShaderManager& operator =(const ShaderManager& _);
};

} // namespace glt

#endif

