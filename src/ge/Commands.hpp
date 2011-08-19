#ifndef GE_COMMANDS_HPP
#define GE_COMMANDS_HPP

#include "ge/Command.hpp"

namespace ge {

struct Commands {
    Ref<Command> printContextInfo;
    Ref<Command> printMemInfo;
    Ref<Command> reloadShaders;
    Ref<Command> listCachedShaders;
    Ref<Command> listBindings;
    Ref<Command> bindKey;
    Ref<Command> help;
    Ref<Command> bindShader;
    Ref<Command> initGLDebug;
    Ref<Command> describe;
    Ref<Command> eval;
    Ref<Command> load;
    Ref<Command> addShaderPath;
    Ref<Command> togglePause;
    Ref<Command> perspectiveProjection;
    Ref<Command> postInit;

    Commands();
};

const Commands& commands();

} // namespace ge

#endif
