#include "ge/CommandArgs.hpp"
#include "ge/Command.hpp"

#include <cstring>

namespace ge {

CommandArg::CommandArg() { memset(this, 0, sizeof *this); }

void CommandArg::free() {
    switch (type) {
    case String: delete string; break;
    case KeyCombo: // TODO: implement delete
    case CommandRef:
        delete command.name;
        delete command.ref;
        if (command.quotation != 0) {
            Quotation& q = *command.quotation;
            for (uint32 i = 0; i < q.size(); ++i)
                for (uint32 j = 0; j < q[i].size(); ++j)
                    q[i][j].free();
            delete command.quotation;
        }
        break;
    }

    memset(this, 0, sizeof *this);
}

} // namespace ge
