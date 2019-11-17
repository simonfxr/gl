#ifndef GE_TOKENIZER_HPP
#define GE_TOKENIZER_HPP

#include "ge/CommandArgs.hpp"

#include "sys/io/Stream.hpp"

#include <vector>

namespace ge {

struct GE_API ParserState
{
    char c{};
    char rawC{};
    std::string filename;

    sys::io::StreamResult in_state;
    sys::io::InStream *in;

    int line = 1;
    int col = 0;

    ParserState(sys::io::InStream &_in, std::string_view fn)
      : filename(fn), in_state(sys::io::StreamResult::OK), in(&_in)
    {}
};

GE_API bool
skipStatement(ParserState &state);

GE_API bool
tokenize(ParserState &state, std::vector<CommandArg> &args);

} // namespace ge

#endif
