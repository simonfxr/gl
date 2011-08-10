#include "ge/CommandParams.hpp"

#ifdef CONST_ARRAY
#define PARAM_ARRAY(...) CONST_ARRAY(ge::CommandParamType, __VA_ARGS__)
#else
#error "CONST_ARRAY not defined"
#endif

#define DEFINE_PARAM_ARRAY(name, ...) DEFINE_CONST_ARRAY(name, ge::CommandParamType, __VA_ARGS__)

namespace ge {

const Array<CommandParamType> NULL_PARAMS(0, 0);

DEFINE_PARAM_ARRAY(KEY_COM_PARAMS, KeyComboParam, CommandParam);

DEFINE_PARAM_ARRAY(NUM_NUM_NUM_PARAMS, NumberParam, NumberParam, NumberParam);

DEFINE_PARAM_ARRAY(COM_PARAMS, CommandParam);

DEFINE_PARAM_ARRAY(LIST_PARAMS, ListParam);

DEFINE_PARAM_ARRAY(NUM_PARAMS, NumberParam);

DEFINE_PARAM_ARRAY(INT_PARAMS, IntegerParam);

DEFINE_PARAM_ARRAY(STR_PARAMS, StringParam);

static void print(const std::string& name, const Array<CommandParamType>& ps) {
    std::cerr << "address: " << &ps << " name: " << name << " size: " << ps.size();
    for (uint32 i = 0; i < ps.size(); ++i)
        std::cerr << " " << ps[i];
    std::cerr << std::endl;
}

#define P_AUX(var) print(AS_STRING(var), var)
#define P(var) P_AUX(CONCAT(var, _PARAMS))

void initParams() {
    // P(KEY_COM);
    // P(NUM_NUM_NUM);
    // P(COM);
    // P(LIST);
    // P(NUM);
    // P(INT);
    // P(STR);
}

} // namespace ge

