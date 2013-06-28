#define DEFINE_GLT_MODULE

#include "glt/glt.hpp"
#include "glt/module.hpp"

#include <assert.h>

namespace glt {

Module *module;

void moduleInit() {
    if (module == 0)
        module = new Module;
}

void moduleExit() {
    assert(module != 0);
    delete module;
}

} // namespace glt
