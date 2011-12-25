#ifndef GE_TOKENIZER_HPP
#define GE_TOKENIZER_HPP

#include "data/Ref.hpp"

#include "ge/CommandArgs.hpp"

#include <istream>
#include <fstream>
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

template<typename S = std::istream>
struct IStreamInput : public Input {
    
    bool close_on_delete;
    S& in;
    
    IStreamInput(S& _in) :
        close_on_delete(true), in(_in)
        {}
    
    ~IStreamInput();
    
    virtual Input::State next(char&);
    virtual void close();
};

struct IFStreamInput : public IStreamInput<std::ifstream> {
    IFStreamInput(std::ifstream& _in) :
        IStreamInput<std::ifstream>(_in) {}
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

    ParseState(const Ref<Input>& _in, const std::string& fn) :
        c('\n'), filename(fn), in_state(Input::OK), in(_in), line(1), col(0) {}
};

bool skipStatement(ParseState& state);

bool tokenize(ParseState& state, std::vector<CommandArg>& args);

Input::State readStream(std::istream&, char&);

template <typename S>
IStreamInput<S>::~IStreamInput() {
    if (close_on_delete) {
        close();
    }
}

template <typename S>
Input::State IStreamInput<S>::next(char& c) {
    return readStream(this->in, c);
}

template <typename S>
void IStreamInput<S>::close() {}

} // namespace ge

#endif
