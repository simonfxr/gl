#ifndef GE_COMMAND_ARGS_HPP
#define GE_COMMAND_ARGS_HPP

#include "ge/KeyBinding.hpp"
#include "ge/conf.hpp"

#include "data/Array.hpp"

#include "sys/io/Stream.hpp"

#include <memory>
#include <string>
#include <vector>

namespace ge {

struct Command;

enum CommandParamType
{
    StringParam,   // "abcd", 'foo'
    IntegerParam,  // 42, 12334, ...
    NumberParam,   // 3.14159
    KeyComboParam, // keycombo: [LShift;+Up;-A], [!Mouse1] ...
    CommandParam,  // commandref: &comname or quotation: { com1 arg1 arg2 ; com2
                   // arg3 ... }
    VarRefParam,   // $var
    AnyParam,      // one parameter of any type
    ListParam // all remaining params, can only appear as the last parameter
};

enum CommandType
{
    String,
    Integer,
    Number,
    KeyCombo,
    CommandRef,
    VarRef,
    Nil
};

struct CommandArg;

typedef std::vector<std::vector<CommandArg>> Quotation;

struct GE_API CommandArg
{

    CommandType type;
    union
    {
        const std::string *string;
        int64_t integer;
        double number;
        struct
        {
            const std::string *name;
            std::shared_ptr<Command> *ref;
            Quotation *quotation;
        } command;
        const std::string *var;
        const KeyBinding *keyBinding;
    };

    void free();
};

static_assert(std::is_pod<CommandArg>::value, "Command Arg has to be POD");

struct GE_API CommandPrettyPrinter
{
    CommandPrettyPrinter();
    ~CommandPrettyPrinter();

    void out(sys::io::OutStream &_out);
    void lineLength(size_t len);
    void blockIndent(size_t indent);
    void ignoreEmptyStatements(bool);

    void print(const KeyBinding &bind);
    void print(const CommandArg &arg, bool first = false);
    void print(const Array<CommandArg> &);
    void print(const std::vector<CommandArg> &);
    void print(const Quotation &);
    void printSpaces(size_t);

    void openQuotation();
    void closeQuotation();

    void flush();

private:
    struct State;
    State *const self;
};

} // namespace ge

#endif
