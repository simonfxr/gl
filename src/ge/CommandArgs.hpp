#ifndef GE_COMMAND_ARGS_HPP
#define GE_COMMAND_ARGS_HPP

#include "bl/array_view.hpp"
#include "bl/compare.hpp"
#include "bl/shared_ptr.hpp"
#include "bl/string.hpp"
#include "bl/vector.hpp"
#include "ge/KeyBinding.hpp"
#include "ge/conf.hpp"
#include "pp/enum.hpp"
#include "pp/pimpl.hpp"
#include "sys/io/Stream.hpp"

namespace ge {

struct Command;

#define GE_COMMAND_PARAM_TYPE_ENUM_DEF(T, V0, V)                               \
    T(CommandParamType,                                                        \
      uint8_t,                                                                 \
      V0(String)  /* "abcd", 'foo' */                                          \
      V(Integer)  /* 42, 12334, ... */                                         \
      V(Number)   /* 3.14159 */                                                \
      V(KeyCombo) /* keycombo: [LShift;+Up;-A], [!Mouse1] ... */               \
      V(Command)  /* commandref: &comname or quotation: { com1 arg1 arg2 ;     \
                          com2 arg3 ... } */                                   \
      V(VarRef)   /* $var */                                                   \
      V(Any)      /* some parameter of any type */                             \
      V(List)     /* all remaining params, can only appear as the last         \
                          parameter */                                         \
    )

#define GE_COMMAND_ARG_TYPE_ENUM_DEF(T, V0, V)                                 \
    T(CommandArgType,                                                          \
      uint8_t,                                                                 \
      V0(String) V(Integer) V(Number) V(KeyCombo) V(CommandRef) V(VarRef)      \
        V(Nil))

PP_DEF_ENUM_WITH_API(GE_API, GE_COMMAND_PARAM_TYPE_ENUM_DEF);
PP_DEF_ENUM_WITH_API(GE_API, GE_COMMAND_ARG_TYPE_ENUM_DEF);

struct CommandArg;

struct CommandValue : bl::comparable<CommandValue>
{
    bl::shared_ptr<const bl::string> name;
    bl::shared_ptr<Command> ref;

    // marker type for overload disambiguation
    struct NamedRef
    {};

    CommandValue(bl::shared_ptr<Command> ref);

    CommandValue(bl::string nm, NamedRef)
      : name(bl::make_shared<const bl::string>(std::move(nm)))
    {}

    static CommandValue makeNamedRef(bl::string nm)
    {
        return CommandValue(std::move(nm), NamedRef{});
    }
};

int
compare(const CommandValue &, const CommandValue &);

using Quotation = bl::vector<bl::vector<CommandArg>>;

struct GE_API CommandArg : bl::comparable<CommandArg>
{
    // marker type for overload disambiguation
    struct CommandArgVar
    {};

    struct CommandNamedRef
    {};

    constexpr CommandArg() : integer(0), _type(CommandArgType::Nil) {}

    explicit CommandArg(int64_t x) : integer(x), _type(CommandArgType::Integer)
    {}

    explicit CommandArg(bl::string x)
      : string(std::move(x)), _type(CommandArgType::String)
    {}

    explicit CommandArg(double x)
      : number(std::move(x)), _type(CommandArgType::Number)
    {}

    explicit CommandArg(bl::shared_ptr<Command> comm)
      : command(std::move(comm)), _type(CommandArgType::CommandRef)
    {}

    explicit CommandArg(KeyBinding kb)
      : keyBinding(std::move(kb)), _type(CommandArgType::KeyCombo)
    {}

    explicit CommandArg(CommandValue comm)
      : command(std::move(comm)), _type(CommandArgType::CommandRef)
    {}

    explicit CommandArg(bl::string var, CommandArgVar)
      : var(std::move(var)), _type(CommandArgType::VarRef)
    {}

    ~CommandArg();

    static CommandArg varRef(bl::string var)
    {
        return CommandArg(std::move(var), CommandArgVar{});
    }

    static CommandArg namedCommandRef(bl::string comm)
    {
        return CommandArg(CommandValue::makeNamedRef(std::move(comm)));
    }

    CommandArg(CommandArg &&);
    CommandArg(const CommandArg &);

    CommandArg &operator=(const CommandArg &);
    CommandArg &operator=(CommandArg &&);

    HU_PURE constexpr CommandArgType type() const noexcept { return _type; }

    void reset(); // reset to Nil

    union
    {
        bl::string string;
        int64_t integer;
        double number;
        CommandValue command;
        bl::string var;
        KeyBinding keyBinding;
    };

private:
    CommandArgType _type;
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
    void print(bl::array_view<const CommandArg>);
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
