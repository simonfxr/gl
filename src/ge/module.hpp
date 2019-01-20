#ifndef GE_MODULE_HPP
#define GE_MODULE_HPP

#include "defs.h"

#include <memory>

#ifdef DEFINE_GE_MODULE
#    define GE_MODULE_ACCESS
#else
#    define GE_MODULE_ACCESS const
#endif

namespace ge {

struct GameWindowInit
{
    GameWindowInit();
    ~GameWindowInit();
};

struct Module
{
    GameWindowInit __game_window_init;
};

extern std::unique_ptr<Module> module;

} // namespace ge

#undef GE_MODULE_ACCESS

#endif
