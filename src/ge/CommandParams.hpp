#ifndef COMMAND_PARAMS_HPP
#define COMMAND_PARAMS_HPP

#include "ge/Command.hpp"

namespace ge {

extern const Array<CommandParamType> NULL_PARAMS;

extern const Array<CommandParamType> KEY_COM_PARAMS;

extern const Array<CommandParamType> NUM_NUM_NUM_PARAMS;

extern const Array<CommandParamType> COM_PARAMS;

extern const Array<CommandParamType> LIST_PARAMS;

extern const Array<CommandParamType> NUM_PARAMS;

extern const Array<CommandParamType> INT_PARAMS;

extern const Array<CommandParamType> STR_PARAMS;

void initParams();

} // namespace ge

#endif

