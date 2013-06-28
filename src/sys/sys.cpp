#define DEFINE_SYS_MODULE

#include "sys/sys.hpp"
#include "sys/module.hpp"

#include <assert.h>

namespace sys {

Module *module;

void moduleInit() {
    if (module == 0)
        module = new Module;
}

void moduleExit() {
    assert(module != 0);
    delete module;
}

} // namespace sys
