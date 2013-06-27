#ifndef COMMAND_PARAMS_HPP
#define COMMAND_PARAMS_HPP

#include "ge/Command.hpp"
#include "data/SharedArray.hpp"

namespace ge {

extern GE_API const SharedArray<CommandParamType> NULL_PARAMS;

extern GE_API const SharedArray<CommandParamType> KEY_COM_PARAMS;

extern GE_API const SharedArray<CommandParamType> NUM_NUM_NUM_PARAMS;

extern GE_API const SharedArray<CommandParamType> COM_PARAMS;

extern GE_API const SharedArray<CommandParamType> LIST_PARAMS;

extern GE_API const SharedArray<CommandParamType> NUM_PARAMS;

extern GE_API const SharedArray<CommandParamType> INT_PARAMS;

extern GE_API const SharedArray<CommandParamType> STR_PARAMS;

extern GE_API const SharedArray<CommandParamType> STR_INT_PARAMS;

GE_API void initParams();

} // namespace ge

#endif

