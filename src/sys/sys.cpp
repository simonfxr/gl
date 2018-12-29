#define DEFINE_SYS_MODULE

#include "sys/sys.hpp"
#include "sys/module.hpp"

namespace sys {

Module *module;

void
moduleInit()
{
    if (module == 0)
        module = new Module;
}

void
moduleExit()
{
    delete module;
    module = 0;
}

} // namespace sys
