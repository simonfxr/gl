#ifndef BL_META_HPP
#define BL_META_HPP

#include "bl/core.hpp"
#include "bl/type_traits.hpp"

namespace bl {

template<typename T, T... indices>
struct integer_sequence
{
    static constexpr size_t size() noexcept { return sizeof...(indices); }
};

template<size_t... Ints>
using index_sequence = integer_sequence<size_t, Ints...>;

#if __has_builtin(__make_integer_seq)

template<class T, T E>
using make_integer_sequence = __make_integer_seq<integer_sequence, T, E>;

#elif GU_COMP_GNULIKE_P

template<class T, T E>
using make_integer_sequence = integer_sequence<T, __integer_pack(E)...>;

#else

template<class T, T E, T... Is>
struct make_integer_sequence_impl
{
    struct base_case
    {
        template<T EE>
        using rec_t = integer_sequence<T, Is...>;
    };
    struct rec_case
    {
        template<T EE>
        using rec_t =
          typename make_integer_sequence_impl<T, EE, EE, Is...>::type;
    };
    using type =
      typename std::conditional_t<E == 0, base_case, rec_case>::template rec_t<
        E - 1>;
};

template<class T, T E>
using make_integer_sequence = typename make_integer_sequence_impl<T, E>::type;

#endif

template<std::size_t N>
using make_index_sequence = make_integer_sequence<std::size_t, N>;

template<class... T>
using index_sequence_for = make_index_sequence<sizeof...(T)>;

template<typename... T>
struct type_seq
{};

namespace detail {
template<size_t I, typename Seq>
struct index_type_seq;

template<typename T, typename... Ts>
struct index_type_seq<0, type_seq<T, Ts...>>
{
    using type = T;
};

template<size_t I, typename T, typename... Ts>
struct index_type_seq<I, type_seq<T, Ts...>>
  : index_type_seq<I - 1, type_seq<Ts...>>
{};

} // namespace detail

template<size_t I, typename Seq>
using index_type_seq = typename detail::index_type_seq<I, Seq>::type;

} // namespace bl

#endif
