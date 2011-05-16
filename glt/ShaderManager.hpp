#ifndef SHADER_MANAGER_HPP
#define SHADER_MANAGER_HPP

#include <ostream>
#include <string>
#include <vector>

#include "glt/Ref.hpp"
#include "glt/ShaderObject.hpp"

#include "fs/fs.hpp"

namespace glt {

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
        fs::MTime mtime;
        std::vector<Ref<CachedShaderObject> > deps;
        std::vector<std::pair<std::string, fs::MTime> > incs;
        CachedShaderObject(ShaderManager& _sm, const std::string& k, const fs::MTime& mt) : sm(_sm), key(k), mtime(mt) {}
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

    Ref<CachedShaderObject> lookupShaderObject(const std::string& file, const fs::MTime& mtime);
    bool removeFromCache(CachedShaderObject& so);
    
    void cacheShaderObject(const Ref<CachedShaderObject>& s);

    void setShaderVersion(uint32 vers, ShaderProfile profile = CoreProfile); // e.g. 330
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

