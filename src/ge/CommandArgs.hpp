#ifndef GE_COMMAND_ARGS_HPP
#define GE_COMMAND_ARGS_HPP

#include "data/Array.hpp"
#include "data/Comparable.hpp"
#include "ge/KeyBinding.hpp"
#include "ge/conf.hpp"
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

struct CommandValue : Comparable<CommandValue>
{
    std::shared_ptr<const std::string> name;
    std::shared_ptr<Command> ref;

    // marker type for overload disambiguation
    struct NamedRef
    {};

    CommandValue(std::shared_ptr<Command> ref);

    CommandValue(std::string nm, NamedRef)
      : name(std::make_shared<const std::string>(std::move(nm)))
    {}

    static CommandValue makeNamedRef(std::string nm)
    {
        return CommandValue(std::move(nm), NamedRef{});
    }
};

int
compare(const CommandValue &, const CommandValue &);

using Quotation = std::vector<std::vector<CommandArg>>;

struct GE_API CommandArg : Comparable<CommandArg>
{
    // marker type for overload disambiguation
    struct CommandArgVar
    {};

    struct CommandNamedRef
    {};

    constexpr CommandArg() : integer(0), _type(Nil) {}

    explicit CommandArg(int64_t x) : integer(x), _type(Integer) {}

    explicit CommandArg(std::string x) : string(std::move(x)), _type(String) {}

    explicit CommandArg(double x) : number(std::move(x)), _type(Number) {}

    explicit CommandArg(std::shared_ptr<Command> comm)
      : command(std::move(comm)), _type(CommandRef)
    {}

    explicit CommandArg(KeyBinding kb)
      : keyBinding(std::move(kb)), _type(KeyCombo)
    {}

    explicit CommandArg(CommandValue comm)
      : command(std::move(comm)), _type(CommandRef)
    {}

    explicit CommandArg(std::string var, CommandArgVar)
      : var(std::move(var)), _type(VarRef)
    {}

    ~CommandArg();

    static CommandArg varRef(std::string var)
    {
        return CommandArg(std::move(var), CommandArgVar{});
    }

    static CommandArg namedCommandRef(std::string comm)
    {
        return CommandArg(CommandValue::makeNamedRef(std::move(comm)));
    }

    CommandArg(CommandArg &&);
    CommandArg(const CommandArg &);

    CommandArg &operator=(const CommandArg &);
    CommandArg &operator=(CommandArg &&);

    CommandType type() const { return _type; }

    void reset(); // reset to Nil

    union
    {
        std::string string;
        int64_t integer;
        double number;
        CommandValue command;
        std::string var;
        KeyBinding keyBinding;
    };

private:
    CommandType _type;
};

int
compare(const CommandArg &, const CommandArg &);

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
    DECLARE_PIMPL(GE_API, self);
};

} // namespace ge

#endif
