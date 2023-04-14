#ifndef GL_MATH_VEC_HPP
#define GL_MATH_VEC_HPP

#include "math/real.hpp"

#include <array>
#include <concepts>
#include <span>
#include <tuple>
#include <type_traits>

#define bl_fwd(lvalue) ::std::forward<decltype(lvalue)>(lvalue)

#define DEF_VEC_OP(op, ...)                                                    \
    friend constexpr auto operator op(vec_type<vec<T, N>> auto &&lhs,          \
                                      a_vec_shape<vec<T, N>> auto &&rhs)       \
      __VA_ARGS__                                                              \
    {                                                                          \
        return vec_map(                                                        \
          [](auto &x, auto &y) { return x op y; }, bl_fwd(lhs), bl_fwd(rhs));  \
    }

#define DEF_VEC_SCALAR_OP(op, ...)                                             \
    friend constexpr auto operator op(vec_type<vec<T, N>> auto &&lhs,          \
                                      vec_scalar_shape<vec<T, N>> auto &&rhs)  \
      __VA_ARGS__                                                              \
    {                                                                          \
        return vec_map([&](auto &&x) { return bl_fwd(x) op rhs; },             \
                       bl_fwd(lhs));                                           \
    }                                                                          \
                                                                               \
    friend constexpr auto operator op(vec_scalar_shape<vec<T, N>> auto &&lhs,  \
                                      vec_type<vec<T, N>> auto &&rhs)          \
      __VA_ARGS__                                                              \
    {                                                                          \
        return vec_map([&](auto &&x) { return lhs op bl_fwd(x); },             \
                       bl_fwd(rhs));                                           \
    }

#define DEF_VEC_MUT_OP(op, ...)                                                \
    template<typename U>                                                       \
    constexpr vec &operator PP_CAT(op, =)(const vec<U, N> &rhs) __VA_ARGS__    \
    {                                                                          \
        vec_for_each([](auto &x, auto &y) { x PP_CAT(op, =) y; },              \
                     bl_fwd(*this),                                            \
                     bl_fwd(rhs));                                             \
        return *this;                                                          \
    }

#define DEF_VEC_SCALAR_MUT_OP(op, ...)                                         \
    template<vec_scalar_of_dim<N> U>                                           \
    constexpr vec &operator PP_CAT(op, =)(const U &rhs) __VA_ARGS__            \
    {                                                                          \
        vec_for_each([&](auto &x) { x PP_CAT(op, =) rhs; }, bl_fwd(*this));    \
        return *this;                                                          \
    }

#define DEF_VEC_OP_BOTH(op, ...)                                               \
    DEF_VEC_OP(op, __VA_ARGS__)                                                \
    DEF_VEC_MUT_OP(op, __VA_ARGS__)

#define DEF_VEC_SCALAR_OP_BOTH(op, ...)                                        \
    DEF_VEC_OP_BOTH(op, __VA_ARGS__)                                           \
    DEF_VEC_SCALAR_OP(op, __VA_ARGS__)                                         \
    DEF_VEC_SCALAR_MUT_OP(op, __VA_ARGS__)

namespace math {

template<typename T, size_t N>
struct vec;

template<typename T, size_t N>
struct mat;

template<typename T>
struct vec_classify : public std::false_type
{
    static inline constexpr size_t vec_dim = 1;
    using component_type = T;
};

template<typename T, size_t N>
struct vec_classify<vec<T, N>> : public std::true_type
{
    static inline constexpr size_t vec_dim = N;
    using component_type = T;
};

template<typename T>
concept a_vec = vec_classify<std::decay_t<T>>::value;

template<typename T, size_t N>
concept vec_of_dim = a_vec<T> && (std::decay_t<T>::dim == N);

template<typename T>
concept vec_scalar = !
a_vec<T>;

template<typename T, size_t N>
concept vec_scalar_of_dim = !
vec_of_dim<T, N>;

template<typename V, typename U>
concept vec_type =
  a_vec<U> && a_vec<V> && std::same_as<std::decay_t<V>, std::decay_t<U>>;

template<typename T, typename V>
concept vec_scalar_shape = a_vec<V> && !
vec_type<T, V>;

template<typename T>
inline constexpr size_t vec_dim = vec_classify<std::decay_t<T>>::vec_dim;

template<typename T, typename V>
concept a_vec_shape = vec_of_dim<T, vec_dim<V>>;

void
vec_map() = delete;

void
vec_for_each() = delete;

struct broadcast_t
{};

template<typename T, size_t N>
struct vec : private std::array<T, N>
{
    static_assert(!std::is_reference_v<T>);

    using Base = std::array<T, N>;

    static inline constexpr size_t dim = N;
    using component_type = T;

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

    template<vec_scalar S>
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
                 (a_vec<U> || a_vec<V>) && vec_dim<U> + vec_dim<V> == N)
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

    template<typename F>
    constexpr auto map(F &&f) const
    {
        return vec_map(bl_fwd(f), *this);
    }

    template<typename F>
    constexpr auto map(F &&f)
    {
        return vec_map(bl_fwd(f), *this);
    }

    template<vec_of_dim<N> U, typename F>
    constexpr auto map(U &&rhs, F &&f) const
    {
        return vec_map(bl_fwd(f), *this, bl_fwd(rhs));
    }

    template<vec_of_dim<N> U, typename F>
    constexpr auto map(U &&rhs, F &&f)
    {
        return vec_map(bl_fwd(f), *this, bl_fwd(rhs));
    }

    template<typename U>
    constexpr auto cast() const
    {
        return map([](auto &x) { return static_cast<U>(x); });
    }

    constexpr vec operator-() const
    {
        return map([](const T &x) { return -x; });
    }

    DEF_VEC_OP_BOTH(+)
    DEF_VEC_OP_BOTH(-)
    DEF_VEC_SCALAR_OP_BOTH(*)
    DEF_VEC_SCALAR_OP_BOTH(/)

    static constexpr vec broadcast(const T &x)
    {
        return vec<T, N>{ broadcast_t{}, x };
    }

    constexpr operator std::span<const T, N>() const noexcept
    {
        return { begin(), N };
    }

    constexpr operator std::span<T, N>() noexcept { return { begin(), N }; }

    friend void swap(vec<T, N> &lhs, vec<T, N> &rhs)
    {
        vec_for_each(
          [](auto &x, auto &y) {
              using std::swap;
              swap(x, y);
          },
          bl_fwd(lhs),
          bl_fwd(rhs));
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
};

#undef DEF_VEC_OP
#undef DEF_VEC_MUT_OP
#undef DEF_VEC_OP_BOTH
#undef DEF_VEC_SCALAR_OP
#undef DEF_VEC_SCALAR_MUT_OP
#undef DEF_VEC_SCALAR_OP_BOTH

template<class T, class... U>
vec(T, U...) -> vec<T, 1 + sizeof...(U)>;

template<typename F, a_vec V, vec_of_dim<std::decay_t<V>::dim>... Vs>
constexpr auto
vec_map(F &&f, V &&v0, Vs &&...vs)
{
    constexpr size_t N = std::decay_t<V>::dim;
    using R =
      vec<std::decay_t<
            std::invoke_result_t<F, decltype(v0[0]), decltype(vs[0])...>>,
          N>;

    return [&]<size_t... I>(std::index_sequence<I...>)->R
    {
        return R{ [&](size_t i) { return f(v0[i], vs[i]...); }(I)... };
    }
    (std::make_index_sequence<N>{});
}

template<typename F, a_vec V, vec_of_dim<std::decay_t<V>::dim>... Vs>
constexpr void
vec_for_each(F &&f, V &&v0, Vs &&...vs)
{
    return [&]<size_t... I>(std::index_sequence<I...>)->void
    {
        ([&](size_t i) { return f(v0[i], vs[i]...); }(I), ...);
    }
    (std::make_index_sequence<std::decay_t<V>::dim>{});
}

template<typename T, typename U, size_t N>
inline constexpr bool
operator==(const vec<T, N> &lhs, const vec<U, N> &rhs)
{
    for (size_t i = 0; i < N; ++i)
        if (lhs[i] != rhs[i])
            return false;
    return true;
}

template<typename T, typename U, size_t N>
inline constexpr bool
operator!=(const vec<T, N> &lhs, const vec<U, N> &rhs)
{
    return !(lhs == rhs);
}

template<a_vec V, typename F>
    requires(vec_dim<V> > 1)
constexpr auto fold(V &&v, F &&f)
{
    if constexpr (vec_dim<V> == 1) {
        return bl_fwd(v)[0];
    } else if constexpr (vec_dim<V> == 2) {
        return bl_fwd(f)(bl_fwd(v)[0], bl_fwd(v)[1]);
    } else {
        auto acc = f(bl_fwd(v)[0], bl_fwd(v)[1]);
        for (size_t i = 2; i < vec_dim<V>; ++i)
            acc = f(std::move(acc), bl_fwd(v)[i]);
        return acc;
    }
}

inline constexpr auto
sum(a_vec auto &&v)
{
    constexpr auto N = vec_dim<decltype(v)>;
    using S = typename std::decay_t<decltype(v)>::component_type;
    if constexpr (N == 0)
        return S{};
    else
        return fold(bl_fwd(v), std::plus<S>{});
}

inline constexpr auto
dot(a_vec auto &&lhs, a_vec_shape<decltype(lhs)> auto &&rhs)
{
    constexpr auto N = vec_dim<decltype(lhs)>;
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
distance(a_vec auto &&lhs, a_vec_shape<decltype(lhs)> auto &&rhs)
{
    return length(bl_fwd(lhs) - bl_fwd(rhs));
}

constexpr auto
inverseDistance(a_vec auto &&lhs, a_vec_shape<decltype(lhs)> auto &&rhs)
{
    return inverseLength(bl_fwd(lhs) - bl_fwd(rhs));
}

constexpr auto
distanceSq(a_vec auto &&lhs, a_vec_shape<decltype(lhs)> auto &&rhs)
{
    return quadrance(bl_fwd(lhs) - bl_fwd(rhs));
}

template<a_vec T, a_vec_shape<T> U, typename V = int>
constexpr auto
reflect(T &&a, U &&n, V &&amp = 1)
{
    return a - n * (2 * bl_fwd(amp) * dot(n, a));
}

constexpr auto
min(a_vec auto &&lhs, a_vec_shape<decltype(lhs)> auto &&rhs)
{
    return bl_fwd(lhs).map(bl_fwd(rhs), [](auto &&x, auto &&y) {
        using std::min;
        return min(bl_fwd(x), bl_fwd(y));
    });
}

constexpr auto
max(a_vec auto &&lhs, a_vec_shape<decltype(lhs)> auto &&rhs)
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
lerp(a_vec auto &&a, a_vec_shape<decltype(a)> auto &&b, auto &&t)
{
    return a + (bl_fwd(b) - a) * bl_fwd(t);
}

constexpr auto
directionFromTo(a_vec auto &&a, a_vec_shape<decltype(a)> auto &&b)
{
    return normalize(bl_fwd(b) - bl_fwd(a));
}

constexpr auto
cos(a_vec auto &&a, a_vec_shape<decltype(a)> auto &&b)
{
    return dot(a, b) / (length(a) * length(b));
}

constexpr auto
projectAlong(a_vec auto &&a, a_vec_shape<decltype(a)> auto &&x)
{
    return dot(bl_fwd(a), x) / quadrance(x) * x;
}

constexpr bool
equal(a_vec auto &&a, a_vec_shape<decltype(a)> auto &&b, auto &&epsi)
{
    for (size_t i = 0; i < vec_dim<decltype(a)>; ++i)
        if (distance(a[i], b[i]) > epsi)
            return false;
    return true;
}

template<typename OStream, typename T, size_t N>
inline OStream &
operator<<(OStream &out, const vec<T, N> &v)
{
    out << "vec" << N << "[";
    bool sep = false;
    for (const auto &x : v) {
        if (sep)
            out << ",";
        out << x;
        sep = true;
    }
    return out << "]";
}

template<size_t I, a_vec V>
    requires(I < vec_dim<V>)
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
