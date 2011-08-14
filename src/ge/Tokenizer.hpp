#ifndef GE_TOKENIZER_HPP
#define GE_TOKENIZER_HPP

#include "ge/CommandArgs.hpp"

#include <istream>
#include <vector>

namespace ge {

struct ParseState {
    char c;
    char rawC;
    std::string filename;
    std::istream& in;
    int line;
    int col;
    bool eof;

    ParseState(std::istream& inp, const std::string& fn) :
        c('\n'), filename(fn), in(inp), line(1), col(0), eof(false) {}
};

void skipLine(ParseState& state);

bool tokenize(ParseState& state, std::vector<CommandArg>& args);

} // namespace ge

#endif
