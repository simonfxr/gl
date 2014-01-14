#ifndef GE_MODULE_HPP
#define GE_MODULE_HPP

#ifdef DEFINE_GE_MODULE
#  define GE_MODULE_ACCESS
#else
#  define GE_MODULE_ACCESS const
#endif

namespace ge {

struct GameWindowInit {
    GameWindowInit();
    ~GameWindowInit();
};

struct KeyBindingState {
    KeyBindingState();
    ~KeyBindingState();
    struct Data;
    Data * const self;
};

struct Module {
    GameWindowInit __game_window_init;
    KeyBindingState key_binding;
};

extern Module * GE_MODULE_ACCESS module;

} // namespace ge

#undef GE_MODULE_ACCESS

#endif
