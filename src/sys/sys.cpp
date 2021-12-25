#define DEFINE_SYS_MODULE

#include "sys/sys.hpp"
#include "sys/module.hpp"

namespace sys {

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

} // namespace sys
