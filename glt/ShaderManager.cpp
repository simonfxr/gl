#include "defs.h"
#include "glt/ShaderManager.hpp"
#include "glt/ShaderProgram.hpp"
#include "glt/ShaderCompiler.hpp"

#include <iostream>
#include <map>
#include <cstdio>

namespace glt {

namespace {

const Ref<ShaderProgram> NULL_PROGRAM_REF;

} // namespace anon

typedef std::map<std::string, Ref<ShaderProgram> > ProgramMap;

struct ShaderManager::Data {
    Verbosity verbosity;
    std::ostream *err;
    std::vector<std::string> shaderDirs;
    ProgramMap programs;
    uint32 shader_version;
    ShaderProfile shader_profile;
    bool cache_so;
    Ref<ShaderCache> globalShaderCache;
    PreprocessorDefinitions globalDefines;
    ShaderCompiler shaderCompiler;

    Data(ShaderManager& self) :
        verbosity(Info),
        err(&std::cerr),
        shaderDirs(),
        programs(),
        shader_version(0),
        shader_profile(CoreProfile),
        cache_so(true),
        globalShaderCache(new ShaderCache),
        globalDefines(),
        shaderCompiler(self)
        {}
};

ShaderManager::ShaderManager() :
    self(new Data(*this))
{
    self->shaderCompiler.init();
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
    ERR(("program not found: " + name).c_str());
    return NULL_PROGRAM_REF;
}

void ShaderManager::addProgram(const std::string& name, Ref<ShaderProgram>& program) {
    ASSERT(program);
    self->programs[name] = program;
}

Ref<ShaderProgram> ShaderManager::declareProgram(const std::string& name) {
    Ref<ShaderProgram> prog(new ShaderProgram(*this));
    self->programs[name] = prog;
    return prog;
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

bool ShaderManager::addShaderDirectory(const std::string& dir, bool check_exists) {
    for (uint32 i = 0; i < self->shaderDirs.size(); ++i)
        if (dir == self->shaderDirs[i])
            return true;

    if (check_exists && !sys::fs::exists(dir, sys::fs::Directory))
        return false;
    
    self->shaderDirs.push_back(dir);
    return true;
}

const std::vector<std::string>& ShaderManager::shaderDirectories() const {
    return self->shaderDirs;
}

bool ShaderManager::cacheShaderObjects() const {
    return self->cache_so;
}

void ShaderManager::cacheShaderObjects(bool docache) {
    if (docache != self->cache_so) {
        self->cache_so = docache;
        if (!docache)
            self->globalShaderCache->flush();
    }
}

const Ref<ShaderCache>& ShaderManager::globalShaderCache() {
    return self->globalShaderCache;
}

ShaderCompiler& ShaderManager::shaderCompiler() {
    return self->shaderCompiler;
}

PreprocessorDefinitions& ShaderManager::globalDefines() {
    return self->globalDefines;
}

const PreprocessorDefinitions& ShaderManager::globalDefines() const {
    return self->globalDefines;
}

} // namespace glt
