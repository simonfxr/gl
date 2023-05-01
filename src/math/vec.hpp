#ifndef GL_MATH_VEC_HPP
#define GL_MATH_VEC_HPP

#include "math/real.hpp"

#include <bl/core.hpp>
#include <bl/functor.hpp>
#include <bl/mp.hpp>
#include <bl/rand.hpp>
#include <bl/string_view.hpp>
#include <bl/swap.hpp>

#include <array>
#include <bl/span.hpp>
#include <concepts>
#include <tuple>
#include <type_traits>

#define MATH_DEF_TENSOR_OP(op, ...)                                            \
    friend constexpr auto operator op(same_tensor<Self> auto &&lhs,            \
                                      same_shape<Self> auto &&rhs) __VA_ARGS__ \
    {                                                                          \
        return bl::map(                                                        \
          [](auto &x, auto &y) { return x op y; }, bl_fwd(lhs), bl_fwd(rhs));  \
    }

#define MATH_DEF_TENSOR_SCALAR_OP(op, ...)                                     \
    friend constexpr auto operator op(same_tensor<Self> auto &&lhs,            \
                                      tensor_scalar_for<Self> auto &&y)        \
      __VA_ARGS__                                                              \
    {                                                                          \
        return bl::map([&](auto &&x) { return bl_fwd(x) op y; }, bl_fwd(lhs)); \
    }                                                                          \
                                                                               \
    friend constexpr auto operator op(tensor_scalar_for<Self> auto &&x,        \
                                      same_tensor<Self> auto &&rhs)            \
      __VA_ARGS__                                                              \
    {                                                                          \
        return bl::map([&](auto &&y) { return x op bl_fwd(y); }, bl_fwd(rhs)); \
    }

#define MATH_DEF_TENSOR_MUT_OP(op, ...)                                        \
    friend constexpr Self &operator PP_CAT(op, =)(                             \
      Self &lhs, same_shape<Self> auto &&rhs) __VA_ARGS__                      \
    {                                                                          \
        bl::for_each([](auto &x, auto &&y) { x PP_CAT(op, =) bl_fwd(y); },     \
                     bl_fwd(lhs),                                              \
                     bl_fwd(rhs));                                             \
        return lhs;                                                            \
    }

#define MATH_DEF_TENSOR_SCALAR_MUT_OP(op, ...)                                 \
    friend constexpr Self &operator PP_CAT(op, =)(                             \
      Self &lhs, tensor_scalar_for<Self> auto &&y) __VA_ARGS__                 \
    {                                                                          \
        bl::for_each([&](auto &x) { x PP_CAT(op, =) y; }, bl_fwd(lhs));        \
        return lhs;                                                            \
    }

#define MATH_DEF_TENSOR_OP_BOTH(op, ...)                                       \
    MATH_DEF_TENSOR_OP(op, __VA_ARGS__)                                        \
    MATH_DEF_TENSOR_MUT_OP(op, __VA_ARGS__)

#define MATH_DEF_TENSOR_SCALAR_OP_BOTH(op, ...)                                \
    MATH_DEF_TENSOR_SCALAR_OP(op, __VA_ARGS__)                                 \
    MATH_DEF_TENSOR_SCALAR_MUT_OP(op, __VA_ARGS__)

namespace math {

template<typename T, size_t N>
struct vec;

template<typename T, size_t N>
struct mat;

template<typename T>
struct tensor_classify
{
    static inline constexpr size_t dim = 1;
    static inline constexpr size_t rank = 0;
    using scalar_type = T;
};

template<typename T, size_t N>
struct tensor_classify<vec<T, N>>
{
    static inline constexpr size_t dim = N;
    static inline constexpr size_t rank = 1;
    using scalar_type = T;
};

template<typename T, size_t N>
struct tensor_classify<mat<T, N>>
{
    static inline constexpr size_t dim = N;
    static inline constexpr size_t rank = 2;
    using scalar_type = T;
};

template<typename T>
concept a_tensor = tensor_classify<std::decay_t<T>>::rank > 0;

template<typename T>
concept a_vec = a_tensor<T> && tensor_classify<std::decay_t<T>>::rank == 1;

template<typename T>
concept a_mat = a_tensor<T> && tensor_classify<std::decay_t<T>>::rank == 2;

template<typename T>
using tensor_scalar_type =
  typename tensor_classify<std::decay_t<T>>::scalar_type;

template<typename T>
inline constexpr size_t tensor_dim = tensor_classify<std::decay_t<T>>::dim;

template<typename T>
inline constexpr size_t tensor_rank = tensor_classify<std::decay_t<T>>::rank;

template<typename T, typename V>
concept tensor_scalar_for =
  a_tensor<V> &&
  (!a_tensor<T> ||
   (a_tensor<T> &&
    std::same_as<typename tensor_classify<T>::scalar_type, std::decay_t<T>>) );

template<typename V, typename T>
concept same_rank = a_tensor<V> && a_tensor<T> &&
                    (tensor_rank<V> == tensor_rank<T>);

template<typename V, typename T>
concept same_shape = same_rank<V, T> && tensor_dim<V> ==
tensor_dim<T>;

template<typename V, typename T>
concept same_vec_shape = same_shape<V, T> && a_vec<V> && a_vec<T>;

template<typename V, typename T>
concept same_tensor =
  same_shape<V, T> && std::same_as<std::decay_t<V>, std::decay_t<T>>;

template<typename V, size_t N>
concept vec_of_dim = a_vec<V> && tensor_dim<V> ==
N;

struct broadcast_t
{};

template<typename T, size_t N>
struct vec : private std::array<T, N>
{
    static_assert(!std::is_reference_v<T>);

    using Base = std::array<T, N>;
    using Self = vec<T, N>;

    static inline constexpr size_t dim = N;
    using scalar_type = T;

    using buffer = T[N];

    using Base::back;
    using Base::begin;
    using Base::cbegin;
    using Base::cend;
    using Base::crbegin;
    using Base::crend;
    using Base::data;
    using Base::empty;
    using Base::end;
    using Base::fill;
    using Base::front;
    using Base::rbegin;
    using Base::rend;
    using Base::size;

    constexpr vec() = default;
    constexpr vec(const vec &) = default;
    constexpr vec(vec &&) noexcept(std::is_nothrow_move_constructible_v<T>) =
      default;
    vec &operator=(const vec &) = default;
    vec &operator=(vec &&) noexcept(std::is_nothrow_move_assignable_v<T>) =
      default;

    /**
     * Construct from excactly N components.
     */
    template<typename... Args>
        requires(N > 1 && sizeof...(Args) == N)
    constexpr vec(Args &&...args) : Base{ static_cast<T>(bl_fwd(args))... }
    {}

    /**
     * Truncation: take the first N components from another vector.
     */
    template<a_vec V>
        requires(std::decay_t<V>::dim > N)
    constexpr explicit vec(V &&vec)
      : Base{ ([&]<size_t... I>(std::index_sequence<I...>) {
          return Base{ static_cast<T>(bl_fwd(vec)[I])... };
      }(std::make_index_sequence<N>{})) }
    {}

    template<tensor_scalar_for<Self> S>
    constexpr explicit vec(S &&arg) : vec{ broadcast_t{}, bl_fwd(arg) }
    {}

    template<typename U>
        requires(!std::is_same_v<std::decay_t<U>, T>)
    constexpr explicit vec(const vec<U, N> &v)
      : Base{ static_cast<Base>(v.template cast<T>()) }
    {}

    template<typename U, typename V>
        requires(!std::is_same_v<std::decay_t<U>, std::in_place_t> &&
                 !std::is_same_v<std::decay_t<U>, broadcast_t> &&
                 (a_vec<U> || a_vec<V>) && tensor_dim<U> + tensor_dim<V> == N)
    constexpr explicit vec(U &&part1, V &&part2)
    {
        size_t j = 0;
        if constexpr (a_vec<U>) {
            for (auto &x : part1)
                operator[](j++) = x;
        } else {
            operator[](j++) = bl_fwd(part1);
        }
        if constexpr (a_vec<V>) {
            for (auto &x : part2)
                operator[](j++) = x;
        } else {
            operator[](j++) = bl_fwd(part2);
        }
    }

    /**
     * Construct from excactly N components
     */
    template<typename... Args>
        requires(sizeof...(Args) == N)
    constexpr explicit(N == 0) vec(std::in_place_t, Args &&...args)
      : Base{ static_cast<T>(bl_fwd(args))... }
    {}

    /**
     * broadcast a single value into N components
     */
    template<typename S>
    constexpr vec(broadcast_t, S &&arg)
      : Base{ ([&]<size_t... I>(std::index_sequence<I...>) {
          return Base{ [&](auto) { return static_cast<T>(arg); }(I)... };
      }(std::make_index_sequence<N>{})) }
    {}

    constexpr auto map(auto &&f) const { return bl::map(bl_fwd(f), *this); }

    constexpr auto map(auto &&f) { return bl::map(bl_fwd(f), *this); }

    constexpr auto map(same_shape<Self> auto &&rhs, auto &&f) const
    {
        return bl::map(bl_fwd(f), *this, bl_fwd(rhs));
    }

    constexpr auto map(same_shape<Self> auto &&rhs, auto &&f)
    {
        return bl::map(bl_fwd(f), *this, bl_fwd(rhs));
    }

    template<typename U>
    constexpr auto cast() const
    {
        return map([](auto &x) { return static_cast<U>(x); });
    }

    friend constexpr auto operator-(same_tensor<Self> auto &&lhs)
    {
        return bl::map([](auto &&x) { return -bl_fwd(x); }, bl_fwd(lhs));
    }

    friend constexpr auto operator~(same_tensor<Self> auto &&lhs)
        requires(std::integral<T>)
    {
        return bl::map([](auto &&x) { return ~bl_fwd(x); }, bl_fwd(lhs));
    }

    MATH_DEF_TENSOR_OP_BOTH(+)
    MATH_DEF_TENSOR_OP_BOTH(-)
    MATH_DEF_TENSOR_OP_BOTH(*)
    MATH_DEF_TENSOR_OP_BOTH(/)
    MATH_DEF_TENSOR_SCALAR_OP_BOTH(+)
    MATH_DEF_TENSOR_SCALAR_OP_BOTH(-)
    MATH_DEF_TENSOR_SCALAR_OP_BOTH(*)
    MATH_DEF_TENSOR_SCALAR_OP_BOTH(/)

    MATH_DEF_TENSOR_OP_BOTH(&, requires(std::integral<T>))
    MATH_DEF_TENSOR_OP_BOTH(|, requires(std::integral<T>))
    MATH_DEF_TENSOR_OP_BOTH(^, requires(std::integral<T>))
    MATH_DEF_TENSOR_SCALAR_OP_BOTH(&, requires(std::integral<T>))
    MATH_DEF_TENSOR_SCALAR_OP_BOTH(|, requires(std::integral<T>))
    MATH_DEF_TENSOR_SCALAR_OP_BOTH(^, requires(std::integral<T>))

    static constexpr vec broadcast(const T &x)
    {
        return vec<T, N>{ broadcast_t{}, x };
    }

    constexpr operator bl::span<const T, N>() const noexcept
    {
        return { begin(), N };
    }

    constexpr operator bl::span<T, N>() noexcept { return { begin(), N }; }

    friend void tag_invoke(bl::swap_f, vec<T, N> &lhs, vec<T, N> &rhs)
    {
        bl::for_each(
          [](auto &x, auto &y) { bl::swap(x, y); }, bl_fwd(lhs), bl_fwd(rhs));
    }

    constexpr T &&operator[](size_t i) &&noexcept
    {
        return static_cast<T &&>(Base::operator[](i));
    }

    constexpr const T &operator[](size_t i) const &&noexcept
    {
        return Base::operator[](i);
    }

    constexpr T &operator[](size_t i) &noexcept { return Base::operator[](i); }

    constexpr const T &operator[](size_t i) const &noexcept
    {
        return Base::operator[](i);
    }

    template<std::integral... I>
        requires(sizeof...(I) != 1)
    constexpr auto operator[](I... i) const noexcept
    {
        return vec<T, sizeof...(I)>{
            std::in_place_t{}, operator[](static_cast<size_t>(i))...
        };
    }

    friend constexpr auto tag_invoke(bl::map_f,
                                     auto &&f,
                                     same_tensor<Self> auto &&v0,
                                     same_shape<Self> auto &&...vs)
    {
        using R = std::decay_t<std::invoke_result_t<decltype(f),
                                                    decltype(v0[0]),
                                                    decltype(vs[0])...>>;
        return [&]<size_t... I>(std::index_sequence<I...>)->vec<R, N>
        {
            return { std::in_place,
                     [&](size_t i) { return f(v0[i], vs[i]...); }(I)... };
        }
        (std::make_index_sequence<N>{});
    }

    friend constexpr void tag_invoke(bl::for_each_f,
                                     auto &&f,
                                     same_tensor<Self> auto &&v0,
                                     same_shape<Self> auto &&...vs)
    {
        [&]<size_t... I>(std::index_sequence<I...>)
        {
            ([&](size_t i) { return f(v0[i], vs[i]...); }(I), ...);
        }
        (std::make_index_sequence<N>{});
    }

    friend constexpr bool operator==(const Self &, const Self &) = default;

    friend constexpr auto operator<=>(const Self &, const Self &) = default;

    friend auto tag_invoke(bl::rand::sampler_f,
                           const auto &sampler,
                           bl::type_proxy<vec<T, N>>,
                           auto &&...args)
    {
        auto s =
          bl::rand::sampler(sampler, bl::type_proxy<T>{}, bl_fwd(args)...);
        using Sampler = std::decay_t<decltype(s)>;
        return vec<Sampler, N>{ broadcast_t{}, bl_fwd(s) };
    }
};

auto
tag_invoke(bl::rand::sample_f, a_tensor auto &&ss, bl::rand::rng auto &&rng)
    requires bl::rand::a_distribution<tensor_scalar_type<decltype(ss)>,
                                      decltype(rng)>
{
    return bl::map([&](auto &&s) { return bl::rand::sample(bl_fwd(s), rng); },
                   bl_fwd(ss));
}

template<typename T, typename... U>
vec(T, U...) -> vec<T, 1 + sizeof...(U)>;

template<a_tensor V, typename F>
    requires(tensor_dim<V> > 1)
constexpr auto fold(V &&v, F &&f)
{
    if constexpr (tensor_dim<V> == 1) {
        return bl_fwd(v)[0];
    } else if constexpr (tensor_dim<V> == 2) {
        return bl_fwd(f)(bl_fwd(v)[0], bl_fwd(v)[1]);
    } else {
        auto acc = f(bl_fwd(v)[0], bl_fwd(v)[1]);
        for (size_t i = 2; i < tensor_dim<V>; ++i)
            acc = f(bl::move(acc), bl_fwd(v)[i]);
        return acc;
    }
}

inline constexpr auto
sum(a_tensor auto &&v)
{
    constexpr auto N = tensor_dim<decltype(v)>;
    using S = tensor_scalar_type<decltype(v)>;
    if constexpr (N == 0)
        return S{};
    else
        return fold(bl_fwd(v), std::plus<S>{});
}

inline constexpr auto
dot(a_vec auto &&lhs, same_vec_shape<decltype(lhs)> auto &&rhs)
{
    constexpr auto N = tensor_dim<decltype(lhs)>;
    static_assert(N > 0, "N not > 0");
    auto ret = lhs[0] * rhs[0];
    for (size_t i = 1; i < N; ++i)
        ret += lhs[i] * rhs[i];
    return ret;
}

inline constexpr auto
cross(vec_of_dim<3> auto &&lhs, vec_of_dim<3> auto &&rhs)
{
    return lhs[1, 2, 0] * rhs[2, 0, 1] - lhs[2, 0, 1] * rhs[1, 2, 0];
}

inline constexpr auto
det(vec_of_dim<2> auto &&lhs, vec_of_dim<2> auto &&rhs)
{
    return lhs[0] * rhs[1] - lhs[1] * rhs[0];
}

constexpr auto
quadrance(const a_vec auto &v)
{
    return dot(v, v);
}

constexpr auto
length(const a_vec auto &v)
{
    return sqrt(quadrance(v));
}

constexpr auto
inverseLength(const a_vec auto &v)
{
    return rsqrt(quadrance(v));
}

constexpr auto
normalize(const a_vec auto &v)
{
    return v * inverseLength(v);
}

constexpr auto
distance(a_vec auto &&lhs, same_vec_shape<decltype(lhs)> auto &&rhs)
{
    return length(bl_fwd(lhs) - bl_fwd(rhs));
}

constexpr auto
inverseDistance(a_vec auto &&lhs, same_vec_shape<decltype(lhs)> auto &&rhs)
{
    return inverseLength(bl_fwd(lhs) - bl_fwd(rhs));
}

constexpr auto
distanceSq(a_vec auto &&lhs, same_vec_shape<decltype(lhs)> auto &&rhs)
{
    return quadrance(bl_fwd(lhs) - bl_fwd(rhs));
}

template<a_vec T, same_vec_shape<T> U, typename V = int>
constexpr auto
reflect(T &&a, U &&n, V &&amp = 1)
{
    return a - n * (2 * bl_fwd(amp) * dot(n, a));
}

constexpr auto
min(a_vec auto &&lhs, same_vec_shape<decltype(lhs)> auto &&rhs)
{
    return bl_fwd(lhs).map(bl_fwd(rhs), [](auto &&x, auto &&y) {
        using std::min;
        return min(bl_fwd(x), bl_fwd(y));
    });
}

constexpr auto
max(a_vec auto &&lhs, same_vec_shape<decltype(lhs)> auto &&rhs)
{
    return bl_fwd(lhs).map(bl_fwd(rhs), [](auto &&x, auto &&y) {
        using std::max;
        return max(bl_fwd(x), bl_fwd(y));
    });
}

constexpr auto
abs(a_vec auto &&v)
{
    return bl_fwd(v).map([](auto &&x) {
        using std::abs;
        return abs(bl_fwd(x));
    });
}

constexpr auto
recip(a_vec auto &&v)
{
    return bl_fwd(v).map([](auto &&x) { return recip(bl_fwd(x)); });
}

constexpr auto
lerp(a_vec auto &&a, same_vec_shape<decltype(a)> auto &&b, auto &&t)
{
    return a + (bl_fwd(b) - a) * bl_fwd(t);
}

constexpr auto
directionFromTo(a_vec auto &&a, same_vec_shape<decltype(a)> auto &&b)
{
    return normalize(bl_fwd(b) - bl_fwd(a));
}

constexpr auto
cos(a_vec auto &&a, same_vec_shape<decltype(a)> auto &&b)
{
    return dot(a, b) / (length(a) * length(b));
}

constexpr auto
projectAlong(a_vec auto &&a, same_vec_shape<decltype(a)> auto &&x)
{
    return dot(bl_fwd(a), x) / quadrance(x) * x;
}

constexpr bool
equal(a_vec auto &&a, same_vec_shape<decltype(a)> auto &&b, auto &&epsi)
{
    for (size_t i = 0; i < tensor_dim<decltype(a)>; ++i)
        if (distance(a[i], b[i]) > epsi)
            return false;
    return true;
}

template<typename OStream, typename T, size_t N>
inline decltype(auto)
operator<<(OStream &&out, const vec<T, N> &v)
{
    using namespace bl::literals;
    out << "vec"_k << N << "["_k;
    bool sep = false;
    for (const auto &x : v) {
        if (sep)
            out << ","_k;
        out << x;
        sep = true;
    }
    return out << "]"_k;
}

template<size_t I, a_vec V>
    requires(I < tensor_dim<V>)
HU_FORCE_INLINE constexpr decltype(auto) get(V &&v)
{
    return bl_fwd(v)[I];
}

} // namespace math

BEGIN_NO_WARN_MISMATCHED_TAGS
namespace std {
template<typename T, size_t N>
struct tuple_size<math::vec<T, N>> : public std::integral_constant<size_t, N>
{};
END_NO_WARN_MISMATCHED_TAGS

template<std::size_t I, class T, std::size_t N>
struct tuple_element<I, math::vec<T, N>>
{
    using type = T;
};

} // namespace std

#endif
