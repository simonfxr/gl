#ifndef GE_COMMAND_DETAIL_HPP
#define GE_COMMAND_DETAIL_HPP

#include "err/err.hpp"
#include "ge/Command.hpp"
#include "ge/CommandArgs.hpp"
#include "ge/Event.hpp"
#include "util/functor_traits.hpp"

#include <tuple>

namespace ge {

struct GE_API CommandEvent;

namespace detail {

template<typename T>
struct always_false : std::false_type
{};

template<typename T>
struct command_param_mapping
{
    // static_assert(always_false<T>::value, "cannot instantiate with this
    // type");
};

HU_PURE_FN inline bool
has_type(CommandParamType pt, CommandArgType t) noexcept
{
    switch (pt.value) {
    case CommandParamType::String:
        return t == CommandArgType::String;
    case CommandParamType::Integer:
        return t == CommandArgType::Integer;
    case CommandParamType::Number:
        return t == CommandArgType::Number;
    case CommandParamType::KeyCombo:
        return t == CommandArgType::KeyCombo;
    case CommandParamType::VarRef:
        return t == CommandArgType::VarRef;
    case CommandParamType::Command:
        return t == CommandArgType::CommandRef;
    case CommandParamType::Any:
        return true;
    case CommandParamType::List:
        UNREACHABLE;
    }
    UNREACHABLE;
}

#define DEF_COMMAND_TYPE_MAPPING(T, fld, ComTy)                                \
    template<>                                                                 \
    struct command_param_mapping<T>                                            \
    {                                                                          \
        static inline constexpr CommandParamType param_type = ComTy;           \
        static void check(const ge::CommandArg &arg)                           \
        {                                                                      \
            ASSERT(has_type(ComTy, arg.type()));                               \
        }                                                                      \
        static const T &unwrap(const ge::CommandArg &arg)                      \
        {                                                                      \
            check(arg);                                                        \
            return arg.fld;                                                    \
        }                                                                      \
        static T &unwrap(ge::CommandArg &arg)                                  \
        {                                                                      \
            check(arg);                                                        \
            return arg.fld;                                                    \
        }                                                                      \
    }

DEF_COMMAND_TYPE_MAPPING(int64_t, integer, CommandParamType::Integer);
DEF_COMMAND_TYPE_MAPPING(double, number, CommandParamType::Number);
DEF_COMMAND_TYPE_MAPPING(std::string, string, CommandParamType::String);
DEF_COMMAND_TYPE_MAPPING(std::shared_ptr<Command>,
                         command.ref,
                         CommandParamType::Command);
DEF_COMMAND_TYPE_MAPPING(KeyBinding, keyBinding, CommandParamType::KeyCombo);

template<>
struct command_param_mapping<ArrayView<const CommandArg>>
{
    static inline constexpr CommandParamType param_type =
      CommandParamType::List;
};

template<>
struct command_param_mapping<CommandArg>
{
    static inline constexpr CommandParamType param_type = CommandParamType::Any;
    static void check() {}
    static const CommandArg &unwrap(const CommandArg &a) { return a; }
    static CommandArg &unwrap(CommandArg &a) { return a; }
};

template<typename T>
using decayed_command_param_mapping = command_param_mapping<std::decay_t<T>>;

template<typename... Ts>
struct CommandArgSeq
{};

template<typename T>
struct CommandArgSeqToTuple
{};

template<typename... Ts>
struct CommandArgSeqToTuple<CommandArgSeq<Ts...>>
{
    using type = std::tuple<Ts...>;
};

template<size_t I, typename Seq>
using command_arg_seq_element =
  std::tuple_element_t<I, typename CommandArgSeqToTuple<Seq>::type>;

template<typename T>
struct Proxy
{};

auto is_var_arg(CommandArgSeq<>) -> std::false_type;

auto is_var_arg(CommandArgSeq<ArrayView<const CommandArg>>) -> std::true_type;

template<typename T, typename... Ts>
auto is_var_arg(CommandArgSeq<T, Ts...>)
  -> decltype(is_var_arg(CommandArgSeq<Ts...>{}));

template<typename F, typename... Ts, size_t... Is>
inline void
invoke_indexed(F &&f,
               CommandArgSeq<Ts...>,
               std::integer_sequence<size_t, Is...>,
               const Event<CommandEvent> &ev,
               ArrayView<const CommandArg> args)
{
    static_assert(sizeof...(Ts) == sizeof...(Is));
    ASSERT(args.size() == sizeof...(Ts));
    f(ev,
      decayed_command_param_mapping<
        command_arg_seq_element<Is,
                                CommandArgSeq<Ts...>>>::unwrap(args[Is])...);
}

template<typename F, typename... Ts, size_t... Is>
inline void
invoke_indexed_vararg(F &&f,
                      CommandArgSeq<Ts...>,
                      std::integer_sequence<size_t, Is...>,
                      const Event<CommandEvent> &ev,
                      ArrayView<const CommandArg> args)
{
    static_assert(sizeof...(Ts) == sizeof...(Is) + 1);
    ASSERT(args.size() >= sizeof...(Is));
    f(ev,
      decayed_command_param_mapping<
        command_arg_seq_element<Is, CommandArgSeq<Ts...>>>::unwrap(args[Is])...,
      args.drop(sizeof...(Is)));
}

template<typename F, bool VarArg, typename... Ts>
inline void
invoke(F &&f,
       std::bool_constant<VarArg>,
       CommandArgSeq<Ts...> arg_seq,
       const Event<CommandEvent> &ev,
       ArrayView<const CommandArg> args)
{
    if constexpr (VarArg) {
        invoke_indexed_vararg(std::forward<F>(f),
                              arg_seq,
                              std::make_index_sequence<sizeof...(Ts) - 1>{},
                              ev,
                              args);
    } else {
        invoke_indexed(std::forward<F>(f),
                       arg_seq,
                       std::index_sequence_for<Ts...>{},
                       ev,
                       args);
    }
}

template<typename FTraits, size_t... Is>
auto make_arg_seq(std::index_sequence<Is...>)
  -> CommandArgSeq<functor_traits_arg_type<FTraits, Is + 1>...>;

template<typename FTraits>
auto
make_arg_seq() -> decltype(make_arg_seq<FTraits>(
  std::make_index_sequence<FTraits::arity - 1>{}));

} // namespace detail

} // namespace ge

#endif
