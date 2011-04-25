#include "defs.h"
#include "glt/ShaderManager.hpp"
#include <iostream>

#include <map>
#include <cstdio>

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

bool readWholeFile(FILE *in, char *& file_contents, uint32& file_size) {
    
    file_contents = 0;
    file_size = 0;
    
    if (in == 0)
        return false;
    if (fseek(in, 0, SEEK_END) == -1)
        return false;
    int64 size = ftell(in);
    if (size < 0)
        return false;
    if (fseek(in, 0, SEEK_SET) == -1)
        return false;
    
    char *contents = new char[size + 1];
    if (fread(contents, size, 1, in) != 1) {
        delete[] contents;
        return false;
    }
    
    contents[size] = '\0';
    file_contents = contents;
    file_size = (uint32) size;
    fclose(in);
    
    return true;
}

} // namespace anon

const Ref<ShaderManager::CachedShaderObject> ShaderManager::EMPTY_CACHE_ENTRY(0);

typedef std::map<std::string, WeakRef<ShaderManager::CachedShaderObject> > ShaderCache;

struct ShaderManager::Data {
    Verbosity verbosity;
    std::ostream *err;
    std::vector<std::string> paths;
    ShaderCache cache;
};

ShaderManager::CachedShaderObject::~CachedShaderObject() {
    sm.self->cache.erase(key);
}

ShaderManager::ShaderManager() :
    self(new Data)
{
    self->err = &std::cerr;
    self->verbosity = Info;
}

ShaderManager::~ShaderManager() {
    delete self;
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

std::string ShaderManager::readFile(const std::string& file, char *& contents, uint32& size) const {

    contents = 0;
    size = 0;
    
    FILE *in = fopen(file.c_str(), "r");
    if (in == 0) {
        err() << "couldnt open " << file << std::endl;
        return "";
    }
    
    if (!readWholeFile(in, contents, size)) {
        if (verbosity() >= ShaderManager::OnlyErrors)
            err() << "couldnt read file: " << file << std::endl;

        return "";
    }

    return file;
}

Ref<ShaderManager::CachedShaderObject> ShaderManager::lookupShaderObject(const std::string& file) const {
    ShaderCache::const_iterator it = self->cache.find(file);
    return it == self->cache.end() ? EMPTY_CACHE_ENTRY : it->second.unweak();
}

void ShaderManager::cacheShaderObject(const Ref<ShaderManager::CachedShaderObject>& entry) {
    DEBUG_ASSERT(entry.ptr() != 0);
    self->cache[entry->key] = entry.weak();
}

} // namespace glt
