#define DEFINE_SYS_MODULE

#include "sys/sys.hpp"
#include "sys/module.hpp"

namespace sys {

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
    delete module;
    module = nullptr;
}

} // namespace sys
