#include "ge/CommandArgs.hpp"
#include "ge/Command.hpp"

#include <cstring>

namespace ge {

CommandArg::CommandArg() { memset(this, 0, sizeof *this); }

void CommandArg::free() {
    
    switch (type) {
    case String: delete string; break;
    case KeyCombo: delete keyBinding; break;
    case CommandRef:
        delete command.name;
        delete command.ref;
        // command.quotation gets deleted by command.ref
        break;
    }

    memset(this, 0, sizeof *this);
}

} // namespace ge
