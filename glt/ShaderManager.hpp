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

    Verbosity verbosity() const;
    void verbosity(Verbosity v);

    std::ostream& err() const;
    void err(std::ostream& out);

    bool addPath(const std::string& directory, bool verify_existence = true);
    const std::vector<std::string>& path() const;

    std::string lookupPath(const std::string& file) const;

    std::string readFileInPath(const std::string& file, char *& contents, uint32& size) const;

private:
    
    struct Data;
    friend struct Data;
    Data * const self;

    ShaderManager(const ShaderManager& _);
    ShaderManager& operator =(const ShaderManager& _);
};

} // namespace glt

#endif

