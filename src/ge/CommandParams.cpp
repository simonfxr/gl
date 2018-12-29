#include "ge/CommandParams.hpp"
#include "data/StaticArray.hpp"

#ifdef CONST_ARRAY
#define PARAM_ARRAY(...) CONST_ARRAY(ge::CommandParamType, __VA_ARGS__)
#else
#error "CONST_ARRAY not defined"
#endif

#define DEFINE_PARAM_ARRAY(name, ...)                                          \
    DEFINE_CONST_ARRAY(name, ge::CommandParamType, __VA_ARGS__)

namespace ge {

const Array<CommandParamType> NULL_PARAMS = ARRAY_INITIALIZER(CommandParamType);

DEFINE_PARAM_ARRAY(KEY_COM_PARAMS, KeyComboParam, CommandParam);

DEFINE_PARAM_ARRAY(NUM_NUM_NUM_PARAMS, NumberParam, NumberParam, NumberParam);

DEFINE_PARAM_ARRAY(COM_PARAMS, CommandParam);

DEFINE_PARAM_ARRAY(LIST_PARAMS, ListParam);

DEFINE_PARAM_ARRAY(NUM_PARAMS, NumberParam);

DEFINE_PARAM_ARRAY(INT_PARAMS, IntegerParam);

DEFINE_PARAM_ARRAY(STR_PARAMS, StringParam);

DEFINE_PARAM_ARRAY(STR_INT_PARAMS, StringParam, IntegerParam);

} // namespace ge
