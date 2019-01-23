#ifndef GE_COMMAND_HPP
#define GE_COMMAND_HPP

#include "bl/array_view.hpp"
#include "bl/string.hpp"
#include "bl/vector.hpp"
#include "ge/CommandArgs.hpp"
#include "ge/Command_detail.hpp"
#include "ge/EngineEvents.hpp"
#include "ge/conf.hpp"
#include "sys/io/Stream.hpp"

namespace ge {

using CommandPtr = bl::shared_ptr<Command>;

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
    const bl::vector<CommandParamType> params;
    bl::string namestr;
    bl::string descr;

public:
    Command(bl::vector<CommandParamType> ps, bl::string name, bl::string desc);

    Command(const Command &) = default;
    Command(Command &&) = default;

    Command &operator=(const Command &) = default;
    Command &operator=(Command &&) = default;

    virtual ~Command() override;
    bl::array_view<const CommandParamType> parameters() const { return params; }
    const bl::string &name() const { return namestr; }
    void name(const bl::string &new_name)
    {
        ASSERT(!new_name.empty());
        namestr = new_name;
    }
    const bl::string &description() const { return descr; }
    bl::string interactiveDescription() const;
    virtual void interactive(const Event<CommandEvent> &ev,
                             bl::array_view<const CommandArg>) = 0;
    void handle(const Event<CommandEvent> &ev) final override;
    virtual QuotationCommand *castToQuotation() { return nullptr; }
};

struct GE_API QuotationCommand : public Command
{
    const bl::unique_ptr<Quotation> quotation;

    QuotationCommand(bl::string_view source,
                     int line,
                     int column,
                     bl::string_view desc,
                     bl::unique_ptr<Quotation> quot);
    ~QuotationCommand() override;

    virtual void interactive(const Event<CommandEvent> &ev,
                             bl::array_view<const CommandArg>) override;
    QuotationCommand *castToQuotation() override { return this; }

    QuotationCommand &operator=(const QuotationCommand &) = delete;
    QuotationCommand(const QuotationCommand &) = delete;
};

template<typename F>
struct FunctorCommand : public Command
{
    FunctorCommand(bl::string name, bl::string descr, F f_)
      : Command(make_params(arg_seq{}), std::move(name), std::move(descr))
      , _f(std::move(f_))
    {}
    ~FunctorCommand() override = default;

    void interactive(const Event<CommandEvent> &ev,
                     bl::array_view<const CommandArg> args) final override
    {
        detail::invoke(_f, is_var_arg_t{}, arg_seq{}, ev, args);
    }

private:
    using ftraits = functor_traits<F>;

    static_assert(std::is_same_v<typename ftraits::result_type, void>);
    static_assert(ftraits::arity > 0);

    using arg_seq = decltype(detail::make_arg_seq<ftraits>());
    using is_var_arg_t = decltype(detail::is_var_arg(arg_seq{}));

    F _f;

    template<typename... Ts>
    static bl::vector<CommandParamType> make_params(
      detail::CommandArgSeq<Ts...>)
    {
        bl::vector<CommandParamType> params;
        params = bl::make_vector<CommandParamType>(
          detail::decayed_command_param_mapping<Ts>::param_type...);
        auto &out = sys::io::stdout();
        out << "make_params: " << __PRETTY_FUNCTION__ << sys::io::endl;
        out << " is_var_arg: " << is_var_arg_t::value << sys::io::endl;
        ;
        out << " params: ";
        for (const auto &param : params) {
            out << to_string(param) << " ";
        }
        out << sys::io::endl;

        return params;
    }
};

template<typename F>
inline bl::shared_ptr<Command>
makeCommand(bl::string name, bl::string descr, F &&f)
{
    // avoid excessive bl::shared_ptr<XX> template instantiations
    auto com = bl::shared_ptr<Command>{
        bl::shared_from_ptr_t{},
        static_cast<Command *>(new FunctorCommand<std::decay_t<F>>(
          std::move(name), std::move(descr), std::forward<F>(f)))
    };

    auto &out = sys::io::stdout();

    out << "makeCommand: " << com->name();
    out << " params: ";
    for (const auto &param : com->parameters()) {
        out << to_string(param) << " ";
    }
    out << sys::io::endl;
    out << sys::io::endl;

    return com;
}

template<typename T, typename... Args>
inline bl::shared_ptr<Command>
makeCommand(bl::string name,
            bl::string descr,
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
