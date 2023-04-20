#ifndef GE_COMMAND_ARGS_HPP
#define GE_COMMAND_ARGS_HPP

#include "ge/KeyBinding.hpp"
#include "ge/conf.hpp"
#include "pp/enum.hpp"
#include "sys/io/Stream.hpp"

#include <bl/new.hpp>

#include <span>
#include <string>
#include <vector>

namespace ge {

struct Command;

#define GE_COMMAND_PARAM_TYPE_ENUM_DEF(T, V)                                   \
    T(CommandParamType,                                                        \
      uint8_t,                                                                 \
      V(String)   /* "abcd", 'foo' */                                          \
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

#define GE_COMMAND_ARG_TYPE_ENUM_DEF(T, V)                                     \
    T(CommandArgType,                                                          \
      uint8_t,                                                                 \
      V(String) V(Integer) V(Number) V(KeyCombo) V(CommandRef) V(VarRef)       \
        V(Nil))

PP_DEF_ENUM_WITH_API(GE_API, GE_COMMAND_PARAM_TYPE_ENUM_DEF);
PP_DEF_ENUM_WITH_API(GE_API, GE_COMMAND_ARG_TYPE_ENUM_DEF);

struct CommandArg;

struct CommandValue
{
    std::shared_ptr<const std::string> name;
    std::shared_ptr<Command> ref;

    // marker type for overload disambiguation
    struct NamedRef
    {};

    CommandValue(std::shared_ptr<Command> com);

    CommandValue(std::string nm, NamedRef)
      : name(std::make_shared<const std::string>(bl::move(nm)))
    {}

    static CommandValue makeNamedRef(std::string nm)
    {
        return CommandValue(bl::move(nm), NamedRef{});
    }

    friend std::weak_ordering operator<=>(const CommandValue &,
                                          const CommandValue &) noexcept;
};

using Quotation = std::vector<std::vector<CommandArg>>;

struct GE_API CommandArg
{
    // marker type for overload disambiguation
    struct CommandArgVar
    {};

    struct CommandNamedRef
    {};

    constexpr CommandArg() : integer(0), _type(CommandArgType::Nil) {}

    explicit CommandArg(int64_t x) : integer(x), _type(CommandArgType::Integer)
    {}

    explicit CommandArg(std::string x)
      : string(bl::move(x)), _type(CommandArgType::String)
    {}

    explicit CommandArg(double x)
      : number(bl::move(x)), _type(CommandArgType::Number)
    {}

    explicit CommandArg(std::shared_ptr<Command> comm)
      : command(bl::move(comm)), _type(CommandArgType::CommandRef)
    {}

    explicit CommandArg(KeyBinding kb)
      : keyBinding(bl::move(kb)), _type(CommandArgType::KeyCombo)
    {}

    explicit CommandArg(CommandValue comm)
      : command(bl::move(comm)), _type(CommandArgType::CommandRef)
    {}

    explicit CommandArg(std::string var, CommandArgVar)
      : var(bl::move(var)), _type(CommandArgType::VarRef)
    {}

    ~CommandArg();

    static CommandArg varRef(std::string var)
    {
        return CommandArg(bl::move(var), CommandArgVar{});
    }

    static CommandArg namedCommandRef(std::string comm)
    {
        return CommandArg(CommandValue::makeNamedRef(bl::move(comm)));
    }

    CommandArg(CommandArg &&) noexcept;
    CommandArg(const CommandArg &);

    CommandArg &operator=(const CommandArg &);
    CommandArg &operator=(CommandArg &&) noexcept;

    HU_PURE_FN constexpr CommandArgType type() const noexcept { return _type; }

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

    friend std::weak_ordering operator<=>(const CommandArg &,
                                          const CommandArg &) noexcept;

private:
    CommandArgType _type;
};

struct GE_API CommandPrettyPrinter
  : private bl::runtime_sized_base<CommandPrettyPrinter>
{
    static CommandPrettyPrinter &construct_at(void *) noexcept;

    void out(sys::io::OutStream &_out);
    void lineLength(size_t len);
    void blockIndent(size_t indent);
    void ignoreEmptyStatements(bool);

    void print(const KeyBinding &bind);
    void print(const CommandArg &arg, bool first = false);
    void print(std::span<const CommandArg>);
    void print(const Quotation &);
    void printSpaces(size_t);

    void openQuotation();
    void closeQuotation();

    void flush();

    BL_DECLARE_RUNTIME_SIZED(CommandPrettyPrinter, Impl);
};

} // namespace ge

#endif
