#define DEFINE_GE_MODULE

#include "ge/ge.hpp"
#include "ge/module.hpp"

#include <new>
#include <assert.h>
#include <cstdlib>
#include <cstring>

namespace ge {

Module *module;

void moduleInit() {
    if (module == 0) {
        void *mem = malloc(sizeof (Module));
        memset(mem, 0, sizeof (Module));
        module = new (mem) Module;
    }
}

void moduleExit() {
    assert(module != 0);
    module->~Module();
    free(module);
    module = 0;
}

} // namespace ge
