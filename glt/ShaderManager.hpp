#ifndef SHADER_MANAGER_HPP
#define SHADER_MANAGER_HPP

namespace glt {

struct ShaderManager {
    
    ShaderManager();
    ~ShaderManager();

private:
    
    struct Data;
    friend struct Data;
    Data * const self;

    ShaderManager(const ShaderManager& _);
    ShaderManager& operator =(const ShaderManager& _);
};

} // namespace glt

#endif

