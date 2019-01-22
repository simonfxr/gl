#define DEFINE_GLT_MODULE

#include "glt/glt.hpp"
#include "glt/module.hpp"

#include <cassert>
#include <memory>

namespace glt {

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

} // namespace glt
