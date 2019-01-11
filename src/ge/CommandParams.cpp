#include "ge/CommandParams.hpp"

#define DEFINE_PARAM_ARRAY(name, ...)                                          \
    const std::vector<CommandParamType> name = { __VA_ARGS__ }

namespace ge {

DEFINE_PARAM_ARRAY(NULL_PARAMS);

DEFINE_PARAM_ARRAY(KEY_COM_PARAMS, KeyComboParam, CommandParam);

DEFINE_PARAM_ARRAY(NUM_NUM_NUM_PARAMS, NumberParam, NumberParam, NumberParam);

DEFINE_PARAM_ARRAY(COM_PARAMS, CommandParam);

DEFINE_PARAM_ARRAY(LIST_PARAMS, ListParam);

DEFINE_PARAM_ARRAY(NUM_PARAMS, NumberParam);

DEFINE_PARAM_ARRAY(INT_PARAMS, IntegerParam);

DEFINE_PARAM_ARRAY(STR_PARAMS, StringParam);

DEFINE_PARAM_ARRAY(STR_INT_PARAMS, StringParam, IntegerParam);

} // namespace ge
