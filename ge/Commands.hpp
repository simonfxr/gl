#ifndef GE_COMMANDS_HPP
#define GE_COMMANDS_HPP

#include "ge/Command.hpp"

namespace ge {

namespace commands {

extern const Ref<Command> printContextInfo;

extern const Ref<Command> reloadShaders;

extern const Ref<Command> listBindings;

extern const Ref<Command> bindKey;

extern const Ref<Command> help;

extern const Ref<Command> bindShader;

extern const Ref<Command> initGLDebug;

extern const Ref<Command> describe;

extern const Ref<Command> eval;

extern const Ref<Command> load;

extern const Ref<Command> addShaderPath;

extern const Ref<Command> togglePause;

extern const Ref<Command> perspectiveProjection;

extern const Ref<Command> postInit;

} // namespace commands

} // namespace ge

#endif
