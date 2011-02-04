#include "defs.h"
#include "glt/ShaderManager.hpp"

#include <iostream>


namespace glt {

// namespace {

struct ShaderManager::Data {
    Verbosity verbosity;
    std::ostream *err;
    std::vector<std::string> includeDirs;
};

// } // namespace anon

ShaderManager::ShaderManager() :
    self(new Data)
{
    self->err = &std::cerr;
    self->verbosity = Info;
}

ShaderManager::~ShaderManager() {
    delete self;
}

ShaderManager::Verbosity ShaderManager::verbosity() {
    return self->verbosity;
}

void ShaderManager::verbosity(ShaderManager::Verbosity v) {
    self->verbosity = v;
}

std::ostream& ShaderManager::err() {
    return *self->err;
}

void ShaderManager::err(std::ostream& err) {
    self->err = &err;
}

bool ShaderManager::addPath(const std::string& dir, bool verify_existence) {
    for (uint32 i = 0; i < self->includeDirs.size(); ++i)
        if (dir == self->includeDirs[i])
            return true;

    if (verify_existence) {
        // TODO: check wether dir exists and is a proper directory
    }
    
    self->includeDirs.push_back(dir);
    return true;
}

const std::vector<std::string>& ShaderManager::path() {
    return self->includeDirs;
}

} // namespace glt
