#define DEFINE_GLT_MODULE

#include "glt/glt.hpp"
#include "glt/module.hpp"

#include <cassert>

namespace glt {

Module *module;

void
moduleInit()
{
    if (module == nullptr)
        module = new Module;
}

void
moduleExit()
{
    assert(module != nullptr);
    delete module;
    module = nullptr;
}

} // namespace glt
