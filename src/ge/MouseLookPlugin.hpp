#ifndef GE_MOUSE_LOOK_PLUGIN_HPP
#define GE_MOUSE_LOOK_PLUGIN_HPP

#include "ge/Plugin.hpp"

namespace ge {

struct GE_API MouseLookPlugin : public Plugin {
    MouseLookPlugin();
    ~MouseLookPlugin();

    void grabMouse(bool grab);
    bool grabMouse() const;

    virtual void registerWith(Engine&) FINAL OVERRIDE;
    virtual void registerCommands(CommandProcessor&) FINAL OVERRIDE;

private:
    struct Data;
    Data * const self;

    MouseLookPlugin(const MouseLookPlugin&);
    MouseLookPlugin& operator =(const MouseLookPlugin&);
};

} // namespace ge

#endif
