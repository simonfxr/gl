#ifndef GE_COMMAND_HPP
#define GE_COMMAND_HPP

#include "ge/CommandArgs.hpp"
#include "ge/Command_detail.hpp"
#include "ge/EngineEvents.hpp"
#include "ge/conf.hpp"

#include <bl/span.hpp>
#include <bl/string.hpp>
#include <bl/vec.hpp>

#include <functional>

namespace ge {

using CommandPtr = std::shared_ptr<Command>;

struct CommandProcessor;
struct QuotationCommand;

struct GE_API CommandEvent : public EngineEvent
{
    CommandEvent(Engine &e, CommandProcessor &proc);
    CommandProcessor &processor;
};

struct GE_API Command : public EventHandler<CommandEvent>
{
private:
    const bl::vec<CommandParamType> params;
    bl::string namestr;
    bl::string descr;

public:
    Command(bl::vec<CommandParamType> ps, bl::string name, bl::string desc);

    Command(const Command &) = default;
    Command(Command &&) = default;

    virtual ~Command() override;
    bl::vec<CommandParamType> parameters() const { return params; }
    const bl::string &name() const { return namestr; }
    void name(const bl::string &new_name)
    {
        ASSERT(!new_name.empty());
        namestr = new_name;
    }
    const bl::string &description() const { return descr; }
    bl::string interactiveDescription() const;
    virtual void interactive(const Event<CommandEvent> &ev,
                             bl::span<const CommandArg>) = 0;
    void handle(const Event<CommandEvent> &ev) final override;
    virtual QuotationCommand *castToQuotation() { return nullptr; }
};

struct GE_API QuotationCommand
  : Command
  , bl::non_copyable
{
    const std::unique_ptr<Quotation> quotation;

    QuotationCommand(bl::string_view source,
                     int line,
                     int column,
                     bl::string_view desc,
                     std::unique_ptr<Quotation> quot);
    ~QuotationCommand() override;

    virtual void interactive(const Event<CommandEvent> &ev,
                             bl::span<const CommandArg>) override;
    QuotationCommand *castToQuotation() override { return this; }
};

template<typename F>
struct FunctorCommand : public Command
{
    FunctorCommand(bl::string name, bl::string descr, F f_)
      : Command(make_params(arg_seq{}), bl::move(name), bl::move(descr))
      , _f(bl::move(f_))
    {}
    ~FunctorCommand() override = default;

    void interactive(const Event<CommandEvent> &ev,
                     bl::span<const CommandArg> args) final override
    {
        invoke(_f, is_var_arg_t{}, arg_seq{}, ev, args);
    }

private:
    using ftraits = functor_traits<F>;

    static_assert(std::is_same_v<typename ftraits::result_type, void>);
    static_assert(ftraits::arity > 0);

    using arg_seq = decltype(detail::make_arg_seq<ftraits>());
    using is_var_arg_t = decltype(detail::is_var_arg(arg_seq{}));

    F _f;

    template<typename... Ts>
    static bl::vec<CommandParamType> make_params(detail::CommandArgSeq<Ts...>)
    {
        return { detail::decayed_command_param_mapping<Ts>::param_type... };
    }
};

template<typename F>
inline std::shared_ptr<Command>
makeCommand(bl::string name, bl::string descr, F &&f)
{
    // avoid excessive std::shared_ptr<XX> template instantiations
    return std::shared_ptr<Command>{ static_cast<Command *>(
      new FunctorCommand<std::decay_t<F>>(
        bl::move(name), bl::move(descr), bl_fwd(f))) };
}

template<typename T, typename... Args>
inline std::shared_ptr<Command>
makeCommand(bl::string name,
            bl::string descr,
            T &receiver,
            void (T::*m)(const Event<CommandEvent> &, Args...))
{
    return makeCommand(
      bl::move(name),
      bl::move(descr),
      [&receiver, m](const Event<CommandEvent> &ev, Args... args) {
          (receiver.*m)(ev, args...);
      });
}

} // namespace ge

#endif
