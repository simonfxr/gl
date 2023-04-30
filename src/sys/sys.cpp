#include "sys/sys.hpp"

#include "sys/module.hpp"

#include <bl/box.hpp>

namespace sys {

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

} // namespace sys
