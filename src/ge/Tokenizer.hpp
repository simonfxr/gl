#ifndef GE_TOKENIZER_HPP
#define GE_TOKENIZER_HPP

#include "data/Ref.hpp"

#include "ge/CommandArgs.hpp"

#include <istream>
#include <vector>

namespace ge {


struct Input {
    
    enum State {
        OK,
        Blocked,
        Eof,
        Error
    };

    virtual ~Input() {}
    virtual State next(char&) = 0;
    virtual void close() = 0;
};

struct IStreamInput : public Input {
    Ref<std::istream> in;
    IStreamInput(const Ref<std::istream>& _in) : in(_in) {}
    Input::State next(char&);
    void close();
};

struct ParseState {
    char c;
    char rawC;
    std::string filename;

    Input::State in_state;
    Ref<Input> in;
    
    int line;
    int col;

    ParseState(Ref<Input>& _in, const std::string& fn) :
        c('\n'), filename(fn), in_state(Input::OK), in(_in), line(1), col(0) {}
};

void skipLine(ParseState& state);

bool tokenize(ParseState& state, std::vector<CommandArg>& args);

} // namespace ge

#endif
