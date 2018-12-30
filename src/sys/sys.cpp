#define DEFINE_SYS_MODULE

#include "sys/sys.hpp"
#include "sys/module.hpp"

namespace sys {

PRAGMA_PUSH_IGNORE_EXIT_TIME_DESTRUCTOR
std::unique_ptr<Module> module;
PRAGMA_POP

void
moduleInit()
{
    if (!module)
        module.reset(new Module);
}

void
moduleExit()
{
    module.reset();
}

} // namespace sys
