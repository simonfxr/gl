#include "ge/Command.hpp"

namespace ge {

const Array<CommandParamType> NULL_PARAMS(0, 0);

const Array<CommandArg> NULL_ARGS(0, 0);

std::string Command::interactiveDescription() const {
    return description();
}

} // namespace ge
