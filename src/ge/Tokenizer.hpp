#ifndef GE_TOKENIZER_HPP
#define GE_TOKENIZER_HPP

#include "ge/CommandArgs.hpp"

#include "sys/io/Stream.hpp"

#include "bl/vector.hpp"

namespace ge {

struct GE_API ParseState
{
    char c;
    char rawC;
    bl::string filename;

    sys::io::StreamResult in_state;
    sys::io::InStream *in;

    int line;
    int col;

    ParseState(sys::io::InStream &_in, bl::string_view fn)
      : c(0)
      , rawC(0)
      , filename(fn)
      , in_state(sys::io::StreamResult::OK)
      , in(&_in)
      , line(1)
      , col(0)
    {}
};

GE_API bool
skipStatement(ParseState &state);

GE_API bool
tokenize(ParseState &state, bl::vector<CommandArg> &args);

} // namespace ge

#endif
