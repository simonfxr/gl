
#include "glt/ShaderManager.hpp"

namespace glt {

struct ShaderManager::Data {

};

ShaderManager::ShaderManager() :
    self(new Data)
{

}

ShaderManager::~ShaderManager() {
    delete self;
}

} // namespace glt
