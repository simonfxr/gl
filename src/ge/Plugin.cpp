#include "ge/Plugin.hpp"

namespace ge {

Plugin::Plugin() = default;

Plugin::~Plugin() = default;

void
Plugin::registerWith(Engine &)
{}

void
Plugin::registerCommands(CommandProcessor &)
{}

PluginSet::~PluginSet() = default;

} // namespace ge
