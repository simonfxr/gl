#include "glt/glt.hpp"

#include "glt/module.hpp"

namespace glt {

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

} // namespace glt
