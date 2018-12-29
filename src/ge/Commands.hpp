#ifndef GE_COMMANDS_HPP
#define GE_COMMANDS_HPP

#include "ge/Command.hpp"

namespace ge {

struct GE_API Commands
{
    CommandPtr printContextInfo;
    CommandPtr printMemInfo;
    CommandPtr reloadShaders;
    CommandPtr listCachedShaders;
    CommandPtr listBindings;
    CommandPtr bindKey;
    CommandPtr help;
    CommandPtr bindShader;
    CommandPtr initGLDebug;
    CommandPtr ignoreGLDebugMessage;
    CommandPtr describe;
    CommandPtr eval;
    CommandPtr load;
    CommandPtr addShaderPath;
    CommandPtr prependShaderPath;
    CommandPtr removeShaderPath;
    CommandPtr togglePause;
    CommandPtr perspectiveProjection;
    CommandPtr postInit;
    CommandPtr startReplServer;
    CommandPtr printGLInstanceStats;

    Commands();
};

GE_API const Commands &
commands();

} // namespace ge

#endif
