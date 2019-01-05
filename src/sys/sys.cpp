#define DEFINE_SYS_MODULE

#include "sys/sys.hpp"
#include "sys/module.hpp"

namespace sys {

BEGIN_NO_WARN_GLOBAL_DESTRUCTOR
std::unique_ptr<Module> module;
END_NO_WARN_GLOBAL_DESTRUCTOR

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
