#ifndef SHADER_MANAGER_HPP
#define SHADER_MANAGER_HPP

#include <ostream>
#include <string>
#include <vector>

#include "glt/Ref.hpp"
#include "glt/ShaderObject.hpp"

namespace glt {

struct ShaderManager {

    enum Verbosity {
        Quiet,
        OnlyErrors,
        Info
    };

    struct CachedShaderObject {
    private:
        ShaderManager& sm;
    public:
        std::string key;
        ShaderObject so;
        std::vector<Ref<CachedShaderObject> > deps;
        CachedShaderObject(ShaderManager& _sm, const std::string& k) : sm(_sm), key(k) {}
        ~CachedShaderObject();
    };

    static const Ref<CachedShaderObject> EMPTY_CACHE_ENTRY;

    ShaderManager();
    ~ShaderManager();

    Verbosity verbosity() const;
    void verbosity(Verbosity v);

    std::ostream& err() const;
    void err(std::ostream& out);

    bool addPath(const std::string& directory, bool verify_existence = true);
    const std::vector<std::string>& path() const;

    std::string lookupPath(const std::string& basename) const;

    Ref<CachedShaderObject> lookupShaderObject(const std::string& file) const;
    
    void cacheShaderObject(const Ref<CachedShaderObject>& s);

    void setShaderVersion(uint32 vers); // e.g. 330
    uint32 shaderVersion();
    
private:
    
    struct Data;
    friend struct Data;
    friend struct CachedShaderObject;
    Data * const self;

    ShaderManager(const ShaderManager& _);
    ShaderManager& operator =(const ShaderManager& _);
};

} // namespace glt

#endif

