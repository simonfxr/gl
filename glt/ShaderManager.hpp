#ifndef SHADER_MANAGER_HPP
#define SHADER_MANAGER_HPP

#include <ostream>
#include <string>
#include <vector>

#include "data/Ref.hpp"
#include "glt/ShaderObject.hpp"

#include "sys/fs/fs.hpp"

namespace glt {

struct ShaderProgram;

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

    struct CachedShaderObject {
    private:
        ShaderManager& sm;
    public:
        std::string key;
        ShaderObject so;
        sys::fs::MTime mtime;
        std::vector<Ref<CachedShaderObject> > deps;
        std::vector<std::pair<std::string, sys::fs::MTime> > incs;
        CachedShaderObject(ShaderManager& _sm, const std::string& k, const sys::fs::MTime& mt) : sm(_sm), key(k), mtime(mt) {}
        ~CachedShaderObject();
    };

    static const Ref<CachedShaderObject> EMPTY_CACHE_ENTRY;

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

    Ref<CachedShaderObject> lookupShaderObject(const std::string& file, const sys::fs::MTime& mtime);
    bool removeFromCache(CachedShaderObject& so);
    
    void cacheShaderObject(Ref<CachedShaderObject>& s);

    void setShaderVersion(uint32 vers /* e.g. 330 */, ShaderProfile profile = CoreProfile);
    uint32 shaderVersion() const;
    ShaderProfile shaderProfile() const;

    bool cacheShaderObjects() const;
    void cacheShaderObjects(bool docache);
    
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

