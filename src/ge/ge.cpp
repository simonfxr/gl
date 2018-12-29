#define DEFINE_GE_MODULE

#include "ge/ge.hpp"
#include "ge/module.hpp"

#include <cassert>
#include <cstring>
#include <new>

namespace ge {

Module *module;

void
moduleInit()
{
    if (module == nullptr) {
        void *mem = new char[sizeof(Module)];
        memset(mem, 0, sizeof(Module));
        module = new (mem) Module;
    }
}

void
moduleExit()
{
    assert(module != nullptr);
    module->~Module();
    delete[] reinterpret_cast<char *>(module);
    module = nullptr;
}

} // namespace ge
