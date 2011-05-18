#include "defs.h"
#include "glt/ShaderManager.hpp"
#include "glt/ShaderProgram.hpp"
#include <iostream>

#include <map>
#include <cstdio>

namespace glt {

extern Ref<ShaderManager::CachedShaderObject> rebuildShaderObject(ShaderManager& self, Ref<ShaderManager::CachedShaderObject>& so);

namespace {

std::string lookupInPath(const std::vector<std::string>& paths, const std::string& file, FILE *& fh) {
    std::string suffix = "/" + file;
    fh = 0;

    for (uint32 i = 0; i < paths.size(); ++i) {
        std::string filename = paths[i] + suffix;
        fh = fopen(filename.c_str(), "r");
        if (fh != 0)
            return filename;
    }

    return "";
}

} // namespace anon

const Ref<ShaderManager::CachedShaderObject> ShaderManager::EMPTY_CACHE_ENTRY(0);

const Ref<ShaderProgram> NULL_PROGRAM_REF(0);

typedef std::map<std::string, WeakRef<ShaderManager::CachedShaderObject> > ShaderCache;

typedef std::map<std::string, Ref<ShaderProgram> > ProgramMap;

struct ShaderManager::Data {
    Verbosity verbosity;
    std::ostream *err;
    std::vector<std::string> paths;
    ShaderCache cache;
    ProgramMap programs;
    uint32 shader_version;
    ShaderProfile shader_profile;
    bool cache_so;

    Ref<CachedShaderObject> rebuildSO(ShaderManager& self, Ref<CachedShaderObject>& so, const fs::MTime& mtime);
};

ShaderManager::CachedShaderObject::~CachedShaderObject() {
    ShaderCache::iterator it = sm.self->cache.find(key);
    if (it != sm.self->cache.end() && it->second.ptr() == this)
        sm.self->cache.erase(it);
}

ShaderManager::ShaderManager() :
    self(new Data)
{
    self->err = &std::cerr;
    self->verbosity = Info;
    self->shader_version = 0;
    self->shader_profile = CoreProfile;
    self->cache_so = true;
}

ShaderManager::~ShaderManager() {
    delete self;
}

    void ShaderManager::setShaderVersion(uint32 vers, ShaderProfile prof) {
    ShaderProfile newprof = CoreProfile;
    if (prof == CompatibilityProfile)
        newprof = CompatibilityProfile;
    self->shader_version = vers;
    self->shader_profile = newprof;
}

uint32 ShaderManager::shaderVersion() const {
    return self->shader_version;
}

ShaderManager::ShaderProfile ShaderManager::shaderProfile() const {
    return self->shader_profile;
}

ShaderManager::Verbosity ShaderManager::verbosity() const {
    return self->verbosity;
}

void ShaderManager::verbosity(ShaderManager::Verbosity v) {
    self->verbosity = v;
}

Ref<ShaderProgram> ShaderManager::program(const std::string& name) const {
    ProgramMap::const_iterator it = self->programs.find(name);
    if (it != self->programs.end())
        return it->second;
    return NULL_PROGRAM_REF;
}

void ShaderManager::addProgram(const std::string& name, Ref<ShaderProgram>& program) {
    ASSERT(program);
    self->programs[name] = program;
}

void ShaderManager::reloadShaders() {
    ProgramMap::iterator it = self->programs.begin();
    for (; it != self->programs.end(); ++it)
        it->second->reload();
}

std::ostream& ShaderManager::err() const {
    return *self->err;
}

void ShaderManager::err(std::ostream& err) {
    self->err = &err;
}

bool ShaderManager::addPath(const std::string& dir, bool verify_existence) {
    for (uint32 i = 0; i < self->paths.size(); ++i)
        if (dir == self->paths[i])
            return true;

    if (verify_existence) {
        // TODO: check wether dir exists and is a proper directory
    }
    
    self->paths.push_back(dir);
    return true;
}

const std::vector<std::string>& ShaderManager::path() const {
    return self->paths;
}

std::string ShaderManager::lookupPath(const std::string& file) const {
    FILE *fh;
    std::string name = lookupInPath(path(), file, fh);
    if (fh != 0)
        fclose(fh);
    return name;
}

Ref<ShaderManager::CachedShaderObject> ShaderManager::Data::rebuildSO(ShaderManager& self, Ref<ShaderManager::CachedShaderObject>& so, const fs::MTime& mtime) {

    bool outdated = false;

    if (mtime != so->mtime) {
        outdated = true;
    } else {
        for (uint32 i = 0; i < so->incs.size(); ++i) {
            fs::MTime mtime = fs::getMTime(so->incs[i].first);
            if (mtime != so->incs[i].second) {
                outdated = true;
                break;
            }
        }
    }

    if (outdated)
        return rebuildShaderObject(self, so);

    Ref<CachedShaderObject> new_so;
    for (uint32 i = 0; i < so->deps.size(); ++i) {
        Ref<CachedShaderObject>& child = so->deps[i];
        Ref<CachedShaderObject> new_child = rebuildSO(self, child, fs::getMTime(child->key));
        if (new_child.ptr() != child.ptr()) {
            if (new_child.ptr() == 0)
                return EMPTY_CACHE_ENTRY;
            
            if (new_so.ptr() == 0) {
                new_so = new CachedShaderObject(self, so->key, mtime);
                new_so->so = so->so;
                so->so.handle = 0;
            }
            
            
            for (uint32 j = new_so->deps.size(); j < i; ++j)
                new_so->deps.push_back(so->deps[j]);
            
            new_so->deps.push_back(new_child);
        }
    }

    return new_so.ptr() == 0 ? so : new_so;
}

Ref<ShaderManager::CachedShaderObject> ShaderManager::lookupShaderObject(const std::string& file, const fs::MTime& mtime) {
    ShaderCache::iterator it = self->cache.find(file);

    if (it != self->cache.end()) {
        Ref<CachedShaderObject> ref = it->second.unweak();
        return self->rebuildSO(*this, ref, mtime);
    }

    return EMPTY_CACHE_ENTRY;
}

bool ShaderManager::removeFromCache(ShaderManager::CachedShaderObject& so) {
    ShaderCache::iterator it = self->cache.find(so.key);
    if (it != self->cache.end() && it->second.ptr() == &so) {
        self->cache.erase(it);
        return true;
    }
    return false;
}

void ShaderManager::cacheShaderObject(const Ref<ShaderManager::CachedShaderObject>& entry) {
    DEBUG_ASSERT(entry.ptr() != 0);
    if (self->cache_so)
        self->cache[entry->key] = entry.weak();
}

bool ShaderManager::cacheShaderObjects() const {
    return self->cache_so;
}

void ShaderManager::cacheShaderObjects(bool docache) {
    if (docache != self->cache_so) {
        self->cache_so = docache;
        if (!docache)
            self->cache.clear();
    }
}

} // namespace glt
