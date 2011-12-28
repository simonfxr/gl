#ifndef GE_COMMAND_ARGS_HPP
#define GE_COMMAND_ARGS_HPP

#include "ge/KeyBinding.hpp"

#include "data/Array.hpp"

#include "sys/io/Stream.hpp"

#include <string>
#include <vector>


template <typename T>
struct Ref;

namespace ge {

using namespace defs;

struct Command;

enum CommandParamType {
    StringParam, // "abcd", 'foo'
    IntegerParam, // 42, 12334, ...
    NumberParam, // 3.14159
    KeyComboParam, // keycombo: [LShift;+Up;-A], [!Mouse1] ...
    CommandParam, // commandref: &comname or quotation: { com1 arg1 arg2 ; com2 arg3 ... }
    VarRefParam, // $var
    AnyParam, // one parameter of any type
    ListParam // all remaining params, can only appear as the last parameter
};

enum CommandType {
    String,
    Integer,
    Number,
    KeyCombo,
    CommandRef,
    VarRef,
    Nil
};

struct CommandArg;

typedef std::vector<std::vector<CommandArg> > Quotation;

struct CommandArg {
    CommandArg();

    CommandType type;
    union {
        const std::string *string;
        int64 integer;
        double number;
        struct {
            const std::string *name;
            Ref<Command> *ref;
            Quotation *quotation;
        } command;
        const std::string *var;
        const KeyBinding *keyBinding;
    };

    void free();
};

void prettyKeyCombo(sys::io::OutStream& out, const KeyBinding& binding);

void prettyCommandArg(sys::io::OutStream& out, const CommandArg& arg);

void prettyCommandArgs(sys::io::OutStream& out, const Array<CommandArg>& args);

} // namepspace ge

#endif
