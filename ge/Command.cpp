#include "ge/Command.hpp"
#include "data/Array.hpp"

#include <sstream>
#include <vector>
#include <iostream>

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

static std::vector<const Array<int> *> *arrays = 0;

static void init() {
    if (arrays == 0) {
        std::cerr << "constructing arrays" << std::endl;
        arrays = new std::vector<const Array<int> *>;
    }
}

extern void register_arr(const Array<int> *arr) {
    init();
    arrays->push_back(arr);
    check_arrs();
}

extern void unregister_arr(const Array<int> *arr) {
    init();
    check_arrs();
    for (uint32 i = 0; i < arrays->size(); ++i) {
        if ((*arrays)[i] == arr) {
            arrays->erase(arrays->begin() + i);
            return;
        }
    }

    ERR("array not found!");
}

extern void check_arrs() {
    init();
    for (uint32 i = 0; i < arrays->size(); ++i) {
        ASSERT((*arrays)[i]->magic1 == VAL);
        ASSERT((*arrays)[i]->magic2 == VAL);
    }
}
