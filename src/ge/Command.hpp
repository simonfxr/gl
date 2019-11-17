#ifndef GE_COMMAND_HPP
#define GE_COMMAND_HPP

#include "ge/CommandArgs.hpp"
#include "ge/Command_detail.hpp"
#include "ge/EngineEvents.hpp"
#include "ge/conf.hpp"
#include "util/ArrayView.hpp"

#include <functional>
#include <memory>
#include <string>
#include <vector>

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
    const std::vector<CommandParamType> params;
    std::string namestr;
    std::string descr;

public:
    Command(std::vector<CommandParamType> ps,
            std::string name,
            std::string desc);

    Command(const Command &) = default;
    Command(Command &&) = default;

    virtual ~Command() override;
    std::vector<CommandParamType> parameters() const { return params; }
    const std::string &name() const { return namestr; }
    void name(const std::string &new_name)
    {
        ASSERT(!new_name.empty());
        namestr = new_name;
    }
    const std::string &description() const { return descr; }
    std::string interactiveDescription() const;
    virtual void interactive(const Event<CommandEvent> &ev,
                             ArrayView<const CommandArg>) = 0;
    void handle(const Event<CommandEvent> &ev) final override;
    virtual QuotationCommand *castToQuotation() { return nullptr; }
};

struct GE_API QuotationCommand
  : Command
  , NonCopyable
{
    const std::unique_ptr<Quotation> quotation;

    QuotationCommand(std::string_view source,
                     int line,
                     int column,
                     std::string_view desc,
                     std::unique_ptr<Quotation> quot);
    ~QuotationCommand() override;

    virtual void interactive(const Event<CommandEvent> &ev,
                             ArrayView<const CommandArg>) override;
    QuotationCommand *castToQuotation() override { return this; }
};

template<typename F>
struct FunctorCommand : public Command
{
    FunctorCommand(std::string name, std::string descr, F f_)
      : Command(make_params(arg_seq{}), std::move(name), std::move(descr))
      , _f(std::move(f_))
    {}
    ~FunctorCommand() override = default;

    void interactive(const Event<CommandEvent> &ev,
                     ArrayView<const CommandArg> args) final override
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
    static std::vector<CommandParamType> make_params(
      detail::CommandArgSeq<Ts...>)
    {
        return { detail::decayed_command_param_mapping<Ts>::param_type... };
    }
};

template<typename F>
inline std::shared_ptr<Command>
makeCommand(std::string name, std::string descr, F &&f)
{
    // avoid excessive std::shared_ptr<XX> template instantiations
    return std::shared_ptr<Command>{ static_cast<Command *>(
      new FunctorCommand<std::decay_t<F>>(
        std::move(name), std::move(descr), std::forward<F>(f))) };
}

template<typename T, typename... Args>
inline std::shared_ptr<Command>
makeCommand(std::string name,
            std::string descr,
            T &receiver,
            void (T::*m)(const Event<CommandEvent> &, Args...))
{
    return makeCommand(
      std::move(name),
      std::move(descr),
      [&receiver, m](const Event<CommandEvent> &ev, Args... args) {
          (receiver.*m)(ev, args...);
      });
}

} // namespace ge

#endif
