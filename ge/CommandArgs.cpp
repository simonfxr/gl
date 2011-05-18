#include "ge/CommandArgs.hpp"

#include <cstring>

namespace ge {

CommandArg::CommandArg() { memset(this, 0, sizeof *this); }

void CommandArg::free() {
    switch (type) {
    case String: delete string; break;
    case KeyCombo: // TODO: implement delete
    case CommandRef: delete command.name;
    }

    memset(this, 0, sizeof *this);
}

} // namespace ge
