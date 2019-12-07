#define DEFINE_GLT_MODULE

#include "glt/glt.hpp"
#include "glt/module.hpp"

#include <cassert>
#include <memory>

namespace glt {

BEGIN_NO_WARN_GLOBAL_DESTRUCTOR
std::unique_ptr<Module> module;
END_NO_WARN_GLOBAL_DESTRUCTOR

void
moduleInit()
{
    if (!module)
        module = std::make_unique<Module>();
}

void
moduleExit()
{
    module.reset();
}

} // namespace glt
