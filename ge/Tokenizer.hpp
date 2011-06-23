#ifndef GE_TOKENIZER_HPP
#define GE_TOKENIZER_HPP

#include "ge/CommandArgs.hpp"
#include "ge/CommandProcessor.hpp"

#include <istream>
#include <vector>

namespace ge {

struct ParseState {
    char c;
    char rawC;
    std::string filename;
    std::istream& in;
    CommandProcessor& proc;
    int line;
    int col;
    bool eof;

    ParseState(std::istream& inp, CommandProcessor& p, const std::string& fn) :
        c('\n'), filename(fn), in(inp), proc(p), line(1), col(0), eof(false) {}
};

void skipLine(ParseState& state);

bool tokenize(ParseState& state, std::vector<CommandArg>& args);

} // namespace ge

#endif
