#ifndef GE_PLUGIN_HPP
#define GE_PLUGIN_HPP

#include "ge/conf.hpp"

namespace ge {

struct GE_API Engine;
struct GE_API CommandProcessor;

struct GE_API Plugin {
    Plugin();
    virtual ~Plugin();

    virtual void registerWith(Engine&) = 0;
    virtual void registerCommands(CommandProcessor&) = 0;
};

} // namespace ge

#endif
