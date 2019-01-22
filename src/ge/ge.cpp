#define DEFINE_GE_MODULE

#include "ge/ge.hpp"
#include "ge/module.hpp"

#include <cassert>
#include <cstring>
#include <new>

namespace ge {

BEGIN_NO_WARN_GLOBAL_DESTRUCTOR
bl::unique_ptr<Module> module;
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

} // namespace ge
