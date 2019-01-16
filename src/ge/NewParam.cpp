#include "data/functor_traits.hpp"
#include "err/err.hpp"
#include "ge/Command.hpp"
#include "ge/CommandArgs.hpp"

#include <tuple>

namespace ge {

template<typename T>
struct always_false : std::false_type
{};

template<typename T>
struct command_param_mapping
{
    // static_assert(always_false<T>::value, "cannot instantiate with this
    // type");
};

inline bool
has_type(CommandParamType pt, CommandType t)
{
    switch (pt) {
    case StringParam:
        return t == String;
    case IntegerParam:
        return t == Integer;
    case NumberParam:
        return t == Number;
    case KeyComboParam:
        return t == KeyCombo;
    case VarRefParam:
        return t == VarRef;
    case CommandParam:
        return t == CommandRef;
    case AnyParam:
        return true;
    case ListParam:
        ASSERT_FAIL();
    }
    CASE_UNREACHABLE;
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

DEF_COMMAND_TYPE_MAPPING(int64_t, integer, IntegerParam);
DEF_COMMAND_TYPE_MAPPING(double, number, NumberParam);
DEF_COMMAND_TYPE_MAPPING(std::string, string, StringParam);
DEF_COMMAND_TYPE_MAPPING(std::shared_ptr<Command>, command.ref, CommandParam);
DEF_COMMAND_TYPE_MAPPING(KeyBinding, keyBinding, KeyComboParam);

template<>
struct command_param_mapping<CommandArg>
{
    static inline constexpr CommandParamType param_type = AnyParam;
    static void check() {}
    static const CommandArg &unwrap(const CommandArg &a) { return a; }
    static CommandArg &unwrap(CommandArg &a) { return a; }
};

template<bool VarArg, CommandParamType... Types>
struct CommandParams
{};

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

auto map_to_command_params(CommandArgSeq<>) -> CommandParams<false>;

template<typename T>
auto map_to_command_params(CommandArgSeq<T>) -> std::enable_if_t<
  std::is_same_v<std::decay_t<T>, ArrayView<const CommandArg>>,
  CommandParams<true>>;

template<typename T, bool VarArg, CommandParamType... CTs>
auto
push(Proxy<T>, CommandParams<VarArg, CTs...>)
  -> CommandParams<VarArg,
                   command_param_mapping<std::decay_t<T>>::param_type,
                   CTs...>;

template<typename T, typename... Ts>
auto
map_to_command_params(CommandArgSeq<T, Ts...>)
  -> decltype(push(Proxy<T>{}, map_to_command_params(CommandArgSeq<Ts...>{})));

using Foo =
  decltype(push(Proxy<int64_t>{}, map_to_command_params(CommandArgSeq<>{})));

template<typename F, typename... Ts, size_t... Is>
void
invoke_indexed(F &&f,
               CommandArgSeq<Ts...>,
               std::integer_sequence<size_t, Is...>,
               ArrayView<const CommandArg> args)
{
    static_assert(sizeof...(Ts) == sizeof...(Is));
    ASSERT(args.size() == sizeof...(Ts));
    f(command_param_mapping<
      command_arg_seq_element<Is, CommandArgSeq<Ts...>>>::unwrap(args[Is])...);
}

template<typename F, typename... Ts, size_t... Is>
void
invoke(F &&f, CommandArgSeq<Ts...>, ArrayView<const CommandArg> args)
{
    invoke_indexed(std::forward<F>(f),
                   CommandArgSeq<Ts...>{},
                   std::index_sequence_for<Ts...>{},
                   args);
}

inline void
foo(ArrayView<const CommandArg> args)
{
    return invoke([]() {}, CommandArgSeq<>{}, args);
}

inline void
foo2(ArrayView<const CommandArg> args)
{
    return invoke(
      [](int64_t, double) {}, CommandArgSeq<int64_t, double>{}, args);
}

static_assert(std::is_same_v<Foo, CommandParams<false, IntegerParam>>);

static_assert(std::is_same_v<decltype(map_to_command_params(CommandArgSeq<>{})),
                             CommandParams<false>>);

static_assert(
  std::is_same_v<decltype(map_to_command_params(
                   CommandArgSeq<int64_t, double &, const std::string &>{})),
                 CommandParams<false, IntegerParam, NumberParam, StringParam>>);

static_assert(
  std::is_same_v<decltype(map_to_command_params(
                   CommandArgSeq<int64_t,
                                 double &,
                                 const std::string &,
                                 ArrayView<const CommandArg>>{})),
                 CommandParams<true, IntegerParam, NumberParam, StringParam>>);

} // namespace ge
