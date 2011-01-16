#include "defs.h"
#include "glt/ShaderManager.hpp"

#include <iostream>


namespace glt {

// namespace {

struct ShaderManager::Data {
    Verbosity verbosity;
    std::ostream *err;
    std::vector<std::string> includeDirs;

    Data(std::ostream& _err) :
        err(&_err)
        {}
};

// } // namespace anon

ShaderManager::ShaderManager() :
    self(new Data(std::cerr))
{
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

void ShaderManager::addIncludeDir(const std::string& dir) {
    for (uint32 i = 0; i < self->includeDirs.size(); ++i)
        if (dir == self->includeDirs[i])
            return;
    self->includeDirs.push_back(dir);
}

const std::vector<std::string>& ShaderManager::includeDirs() {
    return self->includeDirs;
}

} // namespace glt
