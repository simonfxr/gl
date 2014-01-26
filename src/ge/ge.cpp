#define DEFINE_GE_MODULE

#include "ge/ge.hpp"
#include "ge/module.hpp"

#include <new>
#include <assert.h>
#include <cstring>

namespace ge {

Module *module;

void moduleInit() {
    if (module == 0) {
        void *mem = new char[sizeof (Module)];
        memset(mem, 0, sizeof (Module));
        module = new (mem) Module;
    }
}

void moduleExit() {
    assert(module != 0);
    module->~Module();
    delete[] reinterpret_cast<char *>(module);
    module = nullptr;
    
}

} // namespace ge
