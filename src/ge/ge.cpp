#define DEFINE_GE_MODULE

#include "ge/ge.hpp"
#include "ge/module.hpp"

#include <cassert>
#include <cstring>
#include <new>

namespace ge {

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

} // namespace ge
