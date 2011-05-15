#include "defs.h"
#include "glt/ShaderManager.hpp"
#include <iostream>

#include <map>
#include <cstdio>
#include <stack>

namespace glt {

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

typedef std::map<std::string, WeakRef<ShaderManager::CachedShaderObject> > ShaderCache;

struct ShaderManager::Data {
    Verbosity verbosity;
    std::ostream *err;
    std::vector<std::string> paths;
    ShaderCache cache;
    uint32 shader_version;
    ShaderProfile shader_profile;
    bool cache_so;
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

Ref<ShaderManager::CachedShaderObject> ShaderManager::lookupShaderObject(const std::string& file, const fs::MTime& mtimeRoot) const {
    ShaderCache::iterator it = self->cache.find(file);

    if (it != self->cache.end()) {
        Ref<CachedShaderObject> ref = it->second.unweak();
        std::stack<CachedShaderObject *> down;
        down.push(ref.ptr());

        CachedShaderObject *parent = 0;        
        while (!down.empty()) {
            CachedShaderObject *so = down.top();
            down.pop();
            
            fs::MTime mtime;
            if (parent == 0) {
                mtime = mtimeRoot;
            } else {
                mtime = fs::getMTime(so->key);
            }

            if (mtime != so->mtime) {
                std::cerr << so->key << " modified" << std::endl;
                if (parent == 0) {
                    self->cache.erase(it);
                    return EMPTY_CACHE_ENTRY;
                } else {
                    ShaderCache::iterator it = self->cache.find(so->key);
                    if (it != self->cache.end() && it->second.ptr() == so)
                        self->cache.erase(it);
                    return EMPTY_CACHE_ENTRY;
                }
            }    

            for (uint32 i = 0; i < so->deps.size(); ++i)
                down.push(so->deps[i].ptr());
            
            parent = so;
        }

        return ref;
    }

    return EMPTY_CACHE_ENTRY;
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
