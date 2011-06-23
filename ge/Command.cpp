#include "ge/Command.hpp"

#include <sstream>

namespace ge {

const Array<CommandParamType> NULL_PARAMS(0, 0);

const Array<CommandArg> NULL_ARGS(0, 0);

std::string Command::interactiveDescription() const {
    return description();
}

static std::string describe(const std::string& source, int line, int column, const std::string& desc, Quotation *quot) {
    std::ostringstream str;
    str << "<quotation " << source << "@" << line << ":" << column;
    if (!desc.empty())
        str << " " << desc << ">";
    // TODO: extend description with source code of quotation
    UNUSED(quot);
    return str.str();
}

QuotationCommand::QuotationCommand(const std::string& source, int line, int column, const std::string& desc, Quotation *quot) :
    Command(NULL_PARAMS, describe(source, line, column, desc, quot)),
    quotation(quot)
{}

QuotationCommand::~QuotationCommand() {
    Quotation& q = *quotation;
    for (uint32 i = 0; i < q.size(); ++i)
        for (uint32 j = 0; j < q[i].size(); ++j)
            q[i][j].free();
    delete quotation;
}

void QuotationCommand::interactive(const Event<CommandEvent>&, const Array<CommandArg>&) {
    ERR("cannot execute quotation: not yet implemented"); 
}

void QuotationCommand::handle(const Event<CommandEvent>&) {
    ERR("cannot execute quotation: not yet implemented");
}

} // namespace ge
