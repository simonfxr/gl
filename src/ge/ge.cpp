#define DEFINE_GE_MODULE

#include "ge/ge.hpp"
#include "ge/module.hpp"

#include <cassert>
#include <cstring>
#include <memory>

#include <new>

namespace ge {

BEGIN_NO_WARN_GLOBAL_DESTRUCTOR
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
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

} // namespace ge
