#ifndef GE_TOKENIZER_HPP
#define GE_TOKENIZER_HPP

#include "data/Ref.hpp"

#include "ge/CommandArgs.hpp"

#include "sys/io/Stream.hpp"

#include <istream>
#include <fstream>
#include <vector>

namespace ge {

struct ParseState {
    char c;
    char rawC;
    std::string filename;

    sys::io::StreamResult in_state;
    sys::io::InStream *in;
    
    int line;
    int col;

    ParseState(sys::io::InStream& _in, const std::string& fn) :
        c(0), rawC(0),
        filename(fn),
        in_state(sys::io::StreamOK),
        in(&_in),
        line(1), col(0) {}
};

bool skipStatement(ParseState& state);

bool tokenize(ParseState& state, std::vector<CommandArg>& args);

} // namespace ge

#endif
