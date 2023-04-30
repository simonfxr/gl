#include "ge/ge.hpp"

#include "ge/module.hpp"

namespace ge {

BEGIN_NO_WARN_GLOBAL_DESTRUCTOR
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
bl::box<Module> mod;
END_NO_WARN_GLOBAL_DESTRUCTOR

void
moduleInit()
{}

void
moduleExit()
{}

} // namespace ge
