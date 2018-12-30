#ifndef GE_MODULE_HPP
#define GE_MODULE_HPP

#include <memory>

#ifdef DEFINE_GE_MODULE
#define GE_MODULE_ACCESS
#else
#define GE_MODULE_ACCESS const
#endif

namespace ge {

struct GameWindowInit
{
    GameWindowInit();
    ~GameWindowInit();
};

struct KeyBindingState
{
    KeyBindingState();
    struct Data;
    struct DataDeleter
    {
        void operator()(Data *) noexcept;
    };

    const std::unique_ptr<Data, DataDeleter> self;
};

struct Module
{
    GameWindowInit __game_window_init;
    KeyBindingState key_binding;
};

extern std::unique_ptr<Module> module;

} // namespace ge

#undef GE_MODULE_ACCESS

#endif
