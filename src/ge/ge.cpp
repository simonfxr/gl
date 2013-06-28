#define DEFINE_GE_MODULE

#include "ge/ge.hpp"
#include "ge/module.hpp"

#include <assert.h>

namespace ge {

Module *module;

void moduleInit() {
    if (module == 0)
        module = new Module;
}

void moduleExit() {
    assert(module != 0);
    delete module;
    module = 0;
}

} // namespace ge
