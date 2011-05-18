#ifndef GE_COMMAND_ARGS_HPP
#define GE_COMMAND_ARGS_HPP

#include "data/Array.hpp"

#include <string>

namespace ge {

struct Command;

enum CommandParamType {
    StringParam,
    IntegerParam,
    NumberParam,
    CommandParam,
    KeyComboParam,
    AnyParam, // one parameter of any type
    ListParam, // all remaining params, can only appear as the last parameter
};

enum CommandType {
    String,
    Integer,
    Number,
    KeyCombo,
    CommandRef
};

struct CommandArg {
    CommandArg();

    CommandType type;

    union {
        const std::string *string;
        int64 integer;
        double number;
        struct {
            const std::string *name;
            Command *ref;
        } command;
        void *keyCombo;
    };

    void free();
};


} // namepspace ge

#endif
