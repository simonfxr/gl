#ifndef GE_COMMANDS_HPP
#define GE_COMMANDS_HPP

#include "ge/Command.hpp"

namespace ge {

struct GE_API Commands {
    Ref<Command> printContextInfo;
    Ref<Command> printMemInfo;
    Ref<Command> reloadShaders;
    Ref<Command> listCachedShaders;
    Ref<Command> listBindings;
    Ref<Command> bindKey;
    Ref<Command> help;
    Ref<Command> bindShader;
    Ref<Command> initGLDebug;
    Ref<Command> ignoreGLDebugMessage;
    Ref<Command> describe;
    Ref<Command> eval;
    Ref<Command> load;
    Ref<Command> addShaderPath;
    Ref<Command> prependShaderPath;
    Ref<Command> removeShaderPath;
    Ref<Command> togglePause;
    Ref<Command> perspectiveProjection;
    Ref<Command> postInit;
    Ref<Command> startReplServer;
    Ref<Command> printGLInstanceStats;

    Commands();
};

GE_API const Commands& commands();

} // namespace ge

#endif
