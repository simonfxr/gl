#ifndef COMMAND_PARAMS_HPP
#define COMMAND_PARAMS_HPP

#include "ge/Command.hpp"

namespace ge {

extern GE_API const Array<CommandParamType> NULL_PARAMS;

extern GE_API const Array<CommandParamType> KEY_COM_PARAMS;

extern GE_API const Array<CommandParamType> NUM_NUM_NUM_PARAMS;

extern GE_API const Array<CommandParamType> COM_PARAMS;

extern GE_API const Array<CommandParamType> LIST_PARAMS;

extern GE_API const Array<CommandParamType> NUM_PARAMS;

extern GE_API const Array<CommandParamType> INT_PARAMS;

extern GE_API const Array<CommandParamType> STR_PARAMS;

extern GE_API const Array<CommandParamType> STR_INT_PARAMS;

GE_API void initParams();

} // namespace ge

#endif

