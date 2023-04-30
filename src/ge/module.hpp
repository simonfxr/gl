#ifndef GE_MODULE_HPP
#define GE_MODULE_HPP

#include "defs.h"

#include <bl/box.hpp>

namespace ge {

struct GameWindowInit
{
    GameWindowInit();
    ~GameWindowInit();
};

struct Module
{
    GameWindowInit _internal_game_window_init;
};

extern bl::box<Module> mod;

} // namespace ge

#endif
