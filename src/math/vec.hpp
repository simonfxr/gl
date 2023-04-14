#ifndef GL_MATH_GENVEC_HPP
#define GL_MATH_GENVEC_HPP

#include "math/real.hpp"

#include <array>
#include <concepts>
#include <span>
#include <tuple>
#include <type_traits>

#define bl_fwd(lvalue) ::std::forward<decltype(lvalue)>(lvalue)

#define DEF_GENVEC_OP(op)                                                      \
    template<typename U>                                                       \
    constexpr auto operator op(const genvec<U, N> &rhs) const                  \
    {                                                                          \
        return genvec_map([](auto &x, auto &y) { return x op y; },             \
                          std::forward<decltype(*this)>(*this),                \
                          std::forward<decltype(rhs)>(rhs));                   \
    }

#define DEF_GENVEC_SCALAR_OP(op)                                               \
    template<genvec_scalar_of_dim<N> U>                                        \
    constexpr auto operator op(const U &rhs) const                             \
    {                                                                          \
        return genvec_map([&](auto &x) { return x op rhs; },                   \
                          std::forward<decltype(*this)>(*this));               \
    }

#define DEF_GENVEC_MUT_OP(op)                                                  \
    template<typename U>                                                       \
    constexpr genvec &operator PP_CAT(op, =)(const genvec<U, N> &rhs)          \
    {                                                                          \
        genvec_for_each([](auto &x, auto &y) { x PP_CAT(op, =) y; },           \
                        std::forward<decltype(*this)>(*this),                  \
                        std::forward<decltype(rhs)>(rhs));                     \
        return *this;                                                          \
    }

#define DEF_GENVEC_SCALAR_MUT_OP(op)                                           \
    template<genvec_scalar_of_dim<N> U>                                        \
    constexpr genvec &operator PP_CAT(op, =)(const U &rhs)                     \
    {                                                                          \
        genvec_for_each([&](auto &x) { x PP_CAT(op, =) rhs; },                 \
                        std::forward<decltype(*this)>(*this));                 \
        return *this;                                                          \
    }

#define DEF_GENVEC_OP_BOTH(op)                                                 \
    DEF_GENVEC_OP(op)                                                          \
    DEF_GENVEC_MUT_OP(op)

#define DEF_GENVEC_SCALAR_OP_BOTH(op)                                          \
    DEF_GENVEC_SCALAR_OP(op)                                                   \
    DEF_GENVEC_SCALAR_MUT_OP(op)

#define DEF_FIXED_VEC_TYPE(vtype, vname, etype, N)                             \
    using vtype = ::math::genvec<etype, N>;                                    \
                                                                               \
    inline constexpr vtype vname()                                             \
    {                                                                          \
        return vtype::make(args);                                              \
    }                                                                          \
                                                                               \
    inline constexpr vtype vname(const genvec<etype, N - 1> &l, etype r)       \
    {                                                                          \
        return vtype::make();                                                  \
    }                                                                          \
                                                                               \
    template<typename T>                                                       \
    inline constexpr vtype vname(const genvec<T, N> &v)                        \
    {                                                                          \
        return vtype::convert(v);                                              \
    }                                                                          \
                                                                               \
    inline constexpr vtype vname(etype x)                                      \
    {                                                                          \
        return vtype::fill(x);                                                 \
    }

namespace math {

template<typename T, size_t N>
struct genvec;

template<typename T>
struct genvec_classify : public std::false_type
{
    static inline constexpr size_t vec_dim = 1;
    using component_type = T;
};

template<typename T, size_t N>
struct genvec_classify<genvec<T, N>> : public std::true_type
{
    static inline constexpr size_t vec_dim = N;
    using component_type = T;
};

template<typename T>
concept is_genvec = genvec_classify<std::decay_t<T>>::value;

template<typename T, size_t N>
concept genvec_of_dim = is_genvec<T> && (std::decay_t<T>::dim == N);

template<typename T>
concept genvec_scalar = !
is_genvec<T>;

template<typename T, size_t N>
concept genvec_scalar_of_dim = !
genvec_of_dim<T, N>;

template<typename T>
inline constexpr size_t genvec_dim = genvec_classify<std::decay_t<T>>::vec_dim;

template<typename T, typename V>
concept is_genvec_shape = genvec_of_dim<T, genvec_dim<V>>;

void
genvec_map() = delete;

void
genvec_for_each() = delete;

struct broadcast_t
{};

template<typename T, size_t N>
struct genvec : private std::array<T, N>
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

    constexpr genvec() = default;
    constexpr genvec(const genvec &) = default;
    constexpr genvec(genvec &&) noexcept(
      std::is_nothrow_move_constructible_v<T>) = default;
    genvec &operator=(const genvec &) = default;
    genvec &operator=(genvec &&) noexcept(
      std::is_nothrow_move_assignable_v<T>) = default;

    /**
     * Construct from excactly N components.
     */
    template<typename... Args>
        requires(N > 1 && sizeof...(Args) == N)
    constexpr genvec(Args &&...args)
      : Base{ static_cast<T>(std::forward<Args>(args))... }
    {}

    /**
     * Truncation: take the first N components from another vector.
     */
    template<is_genvec V>
        requires(std::decay_t<V>::dim > N)
    constexpr explicit genvec(V &&vec)
      : Base{ ([&]<size_t... I>(std::index_sequence<I...>) {
          return Base{ static_cast<T>(std::forward<V>(vec)[I])... };
      }(std::make_index_sequence<N>{})) }
    {}

    template<genvec_scalar S>
    constexpr explicit genvec(S &&arg)
      : genvec{ broadcast_t{}, std::forward<S>(arg) }
    {}

    template<typename U>
        requires(!std::is_same_v<std::decay_t<U>, T>)
    constexpr explicit genvec(const genvec<U, N> &v)
      : Base{ static_cast<Base>(v.template cast<T>()) }
    {}

    template<typename U, typename V>
        requires(!std::is_same_v<std::decay_t<U>, std::in_place_t> &&
                 !std::is_same_v<std::decay_t<U>, broadcast_t> &&
                 (is_genvec<U> || is_genvec<V>) &&
                 genvec_dim<U> + genvec_dim<V> == N)
    constexpr explicit genvec(U &&part1, V &&part2)
    {
        size_t j = 0;
        if constexpr (is_genvec<U>) {
            for (auto &x : part1)
                operator[](j++) = x;
        } else {
            operator[](j++) = std::forward<U>(part1);
        }
        if constexpr (is_genvec<V>) {
            for (auto &x : part2)
                operator[](j++) = x;
        } else {
            operator[](j++) = std::forward<V>(part2);
        }
    }

    /**
     * Construct from excactly N components
     */
    template<typename... Args>
        requires(sizeof...(Args) == N)
    constexpr explicit(N == 0) genvec(std::in_place_t, Args &&...args)
      : Base{ static_cast<T>(std::forward<Args>(args))... }
    {}

    /**
     * broadcast a single value into N components
     */
    template<typename S>
    constexpr genvec(broadcast_t, S &&arg)
      : Base{ ([&]<size_t... I>(std::index_sequence<I...>) {
          return Base{ [&](auto) { return static_cast<T>(arg); }(I)... };
      }(std::make_index_sequence<N>{})) }
    {}

    template<typename F>
    constexpr auto map(F &&f) const
    {
        return genvec_map(std::forward<F>(f), *this);
    }

    template<typename F>
    constexpr auto map(F &&f)
    {
        return genvec_map(std::forward<F>(f), *this);
    }

    template<genvec_of_dim<N> U, typename F>
    constexpr auto map(U &&rhs, F &&f) const
    {
        return genvec_map(std::forward<F>(f), *this, std::forward<U>(rhs));
    }

    template<genvec_of_dim<N> U, typename F>
    constexpr auto map(U &&rhs, F &&f)
    {
        return genvec_map(std::forward<F>(f), *this, std::forward<U>(rhs));
    }

    template<typename U>
    constexpr auto cast() const
    {
        return map([](auto &x) { return static_cast<U>(x); });
    }

    constexpr genvec operator-() const
    {
        return map([](const T &x) { return -x; });
    }

    DEF_GENVEC_OP_BOTH(+)
    DEF_GENVEC_OP_BOTH(-)
    DEF_GENVEC_OP_BOTH(*)
    DEF_GENVEC_OP_BOTH(/)

    DEF_GENVEC_SCALAR_OP_BOTH(/)
    DEF_GENVEC_SCALAR_OP_BOTH(*)

    template<typename... Args>
    static constexpr genvec make(Args &&...args)
    {
        static_assert(sizeof...(args) == N,
                      "can only initialize using exactly N elements");
        return { std::in_place, T{ std::forward<Args>(args) }... };
    }

    static constexpr genvec broadcast(const T &x)
    {
        return genvec<T, N>{ broadcast_t{}, x };
    }

    constexpr operator std::span<const T, N>() const noexcept
    {
        return { begin(), N };
    }

    constexpr operator std::span<T, N>() noexcept { return { begin(), N }; }

    friend void swap(genvec<T, N> &lhs, genvec<T, N> &rhs)
    {
        genvec_for_each(
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
        return genvec<T, sizeof...(I)>{
            std::in_place_t{}, operator[](static_cast<size_t>(i))...
        };
    }
};

#undef DEF_GENVEC_OP
#undef DEF_GENVEC_MUT_OP
#undef DEF_GENVEC_OP_BOTH
#undef DEF_GENVEC_SCALAR_OP
#undef DEF_GENVEC_SCALAR_MUT_OP
#undef DEF_GENVEC_SCALAR_OP_BOTH

template<class T, class... U>
genvec(T, U...) -> genvec<T, 1 + sizeof...(U)>;

template<typename F, is_genvec V, genvec_of_dim<std::decay_t<V>::dim>... Vs>
constexpr auto
genvec_map(F &&f, V &&v0, Vs &&...vs)
{
    constexpr size_t N = std::decay_t<V>::dim;
    using R =
      genvec<std::decay_t<
               std::invoke_result_t<F, decltype(v0[0]), decltype(vs[0])...>>,
             N>;

    return [&]<size_t... I>(std::index_sequence<I...>)->R
    {
        return R{ [&](size_t i) { return f(v0[i], vs[i]...); }(I)... };
    }
    (std::make_index_sequence<N>{});
}

template<typename F, is_genvec V, genvec_of_dim<std::decay_t<V>::dim>... Vs>
constexpr void
genvec_for_each(F &&f, V &&v0, Vs &&...vs)
{
    return [&]<size_t... I>(std::index_sequence<I...>)->void
    {
        ([&](size_t i) { return f(v0[i], vs[i]...); }(I), ...);
    }
    (std::make_index_sequence<std::decay_t<V>::dim>{});
}

template<typename T, typename U, size_t N>
    requires(genvec_scalar_of_dim<T, N>)
inline constexpr auto
operator*(const T &lhs, const genvec<U, N> &rhs)
{
    return rhs.map([&](const U &x) { return lhs * x; });
}

template<typename T, typename U, size_t N>
    requires(genvec_scalar_of_dim<T, N>)
inline constexpr auto
operator/(const T &lhs, const genvec<U, N> &rhs)
{
    return rhs.map([&](const U &x) { return lhs / x; });
}

template<typename T, typename U, size_t N>
inline constexpr bool
operator==(const genvec<T, N> &lhs, const genvec<U, N> &rhs)
{
    for (size_t i = 0; i < N; ++i)
        if (lhs[i] != rhs[i])
            return false;
    return true;
}

template<typename T, typename U, size_t N>
inline constexpr bool
operator!=(const genvec<T, N> &lhs, const genvec<U, N> &rhs)
{
    return !(lhs == rhs);
}

template<is_genvec V, typename F>
    requires(genvec_dim<V> > 1)
constexpr auto fold(V &&v, F &&f)
{
    if constexpr (genvec_dim<V> == 1) {
        return bl_fwd(v)[0];
    } else if constexpr (genvec_dim<V> == 2) {
        return bl_fwd(f)(bl_fwd(v)[0], bl_fwd(v)[1]);
    } else {
        auto acc = f(bl_fwd(v)[0], bl_fwd(v)[1]);
        for (size_t i = 2; i < genvec_dim<V>; ++i)
            acc = f(std::move(acc), bl_fwd(v)[i]);
        return acc;
    }
}

inline constexpr auto
sum(is_genvec auto &&v)
{
    constexpr auto N = genvec_dim<decltype(v)>;
    using S = typename std::decay_t<decltype(v)>::component_type;
    if constexpr (N == 0)
        return S{};
    else
        return fold(bl_fwd(v), std::plus<S>{});
}

inline constexpr auto
dot(is_genvec auto &&lhs, is_genvec_shape<decltype(lhs)> auto &&rhs)
{
    constexpr auto N = genvec_dim<decltype(lhs)>;
    static_assert(N > 0, "N not > 0");
    auto ret = lhs[0] * rhs[0];
    for (size_t i = 1; i < N; ++i)
        ret += lhs[i] * rhs[i];
    return ret;
}

inline constexpr auto
cross(genvec_of_dim<3> auto &&lhs, genvec_of_dim<3> auto &&rhs)
{
    return lhs[1, 2, 0] * rhs[2, 0, 1] - lhs[2, 0, 1] * rhs[1, 2, 0];
}

inline constexpr auto
det(genvec_of_dim<2> auto &&lhs, genvec_of_dim<2> auto &&rhs)
{
    return lhs[0] * rhs[1] - lhs[1] * rhs[0];
}

constexpr auto
quadrance(const is_genvec auto &v)
{
    return dot(v, v);
}

constexpr auto
length(const is_genvec auto &v)
{
    return sqrt(quadrance(v));
}

constexpr auto
inverseLength(const is_genvec auto &v)
{
    return rsqrt(quadrance(v));
}

constexpr auto
normalize(const is_genvec auto &v)
{
    return v * inverseLength(v);
}

constexpr auto
distance(is_genvec auto &&lhs, is_genvec_shape<decltype(lhs)> auto &&rhs)
{
    return length(bl_fwd(lhs) - bl_fwd(rhs));
}

constexpr auto
inverseDistance(is_genvec auto &&lhs, is_genvec_shape<decltype(lhs)> auto &&rhs)
{
    return inverseLength(bl_fwd(lhs) - bl_fwd(rhs));
}

constexpr auto
distanceSq(is_genvec auto &&lhs, is_genvec_shape<decltype(lhs)> auto &&rhs)
{
    return quadrance(bl_fwd(lhs) - bl_fwd(rhs));
}

template<is_genvec T, is_genvec_shape<T> U, typename V = int>
constexpr auto
reflect(T &&a, U &&n, V &&amp = 1)
{
    return a - n * (2 * bl_fwd(amp) * dot(n, a));
}

constexpr auto
min(is_genvec auto &&lhs, is_genvec_shape<decltype(lhs)> auto &&rhs)
{
    return bl_fwd(lhs).map(bl_fwd(rhs), [](auto &&x, auto &&y) {
        using std::min;
        return min(bl_fwd(x), bl_fwd(y));
    });
}

constexpr auto
max(is_genvec auto &&lhs, is_genvec_shape<decltype(lhs)> auto &&rhs)
{
    return bl_fwd(lhs).map(bl_fwd(rhs), [](auto &&x, auto &&y) {
        using std::max;
        return max(bl_fwd(x), bl_fwd(y));
    });
}

constexpr auto
abs(is_genvec auto &&v)
{
    return bl_fwd(v).map([](auto &&x) {
        using std::abs;
        return abs(bl_fwd(x));
    });
}

constexpr auto
recip(is_genvec auto &&v)
{
    return bl_fwd(v).map([](auto &&x) { return recip(bl_fwd(x)); });
}

constexpr auto
lerp(is_genvec auto &&a, is_genvec_shape<decltype(a)> auto &&b, auto &&t)
{
    return a + (bl_fwd(b) - a) * bl_fwd(t);
}

constexpr auto
directionFromTo(is_genvec auto &&a, is_genvec_shape<decltype(a)> auto &&b)
{
    return normalize(bl_fwd(b) - bl_fwd(a));
}

constexpr auto
cos(is_genvec auto &&a, is_genvec_shape<decltype(a)> auto &&b)
{
    return dot(a, b) / (length(a) * length(b));
}

constexpr auto
projectAlong(is_genvec auto &&a, is_genvec_shape<decltype(a)> auto &&x)
{
    return dot(bl_fwd(a), x) / quadrance(x) * x;
}

constexpr bool
equal(is_genvec auto &&a, is_genvec_shape<decltype(a)> auto &&b, auto &&epsi)
{
    for (size_t i = 0; i < genvec_dim<decltype(a)>; ++i)
        if (distance(a[i], b[i]) > epsi)
            return false;
    return true;
}

template<typename OStream, typename T, size_t N>
inline OStream &
operator<<(OStream &out, const genvec<T, N> &v)
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

template<size_t I, typename T, size_t N>
HU_FORCE_INLINE inline const T &
get(const math::genvec<T, N> &v)
{
    static_assert(I < N);
    return v[I];
}

template<size_t I, typename T, size_t N>
HU_FORCE_INLINE inline T
get(math::genvec<T, N> &&v)
{
    static_assert(I < N);
    return std::move(v[I]);
}

template<size_t I, typename T, size_t N>
HU_FORCE_INLINE inline T &
get(math::genvec<T, N> &v)
{
    static_assert(I < N);
    return v[I];
}

} // namespace math

BEGIN_NO_WARN_MISMATCHED_TAGS
namespace std {
template<typename T, size_t N>
struct tuple_size<math::genvec<T, N>> : public std::integral_constant<size_t, N>
{};
END_NO_WARN_MISMATCHED_TAGS

template<std::size_t I, class T, std::size_t N>
struct tuple_element<I, math::genvec<T, N>>
{
    using type = T;
};

} // namespace std

#endif
