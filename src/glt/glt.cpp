#define DEFINE_GLT_MODULE

#include "glt/glt.hpp"
#include "glt/module.hpp"

#include <cassert>
#include <memory>

namespace glt {

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

} // namespace glt
