#ifndef GL_MATH_GENVEC_HPP
#define GL_MATH_GENVEC_HPP

#include "math/real.hpp"

#include <array>
#include <concepts>
#include <span>
#include <tuple>
#include <type_traits>

#define DEF_GENVEC_OP(op)                                                      \
    template<typename U>                                                       \
    constexpr auto operator op(const genvec<U, N> &rhs) const                  \
    {                                                                          \
        return genvec_map([](auto &x, auto &y) { return x op y; },             \
                          std::forward<decltype(*this)>(*this),                \
                          std::forward<decltype(rhs)>(rhs));                   \
    }

#define DEF_GENVEC_SCALAR_OP(op)                                               \
    template<is_genvec_scalar_dim<N> U>                                        \
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
    template<is_genvec_scalar_dim<N> U>                                        \
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
struct is_genvec_helper : public std::false_type
{};

template<typename T, size_t N>
struct is_genvec_helper<genvec<T, N>> : public std::true_type
{};

template<typename T>
concept is_genvec = is_genvec_helper<std::decay_t<T>>::value;

template<typename T, size_t N>
concept is_genvec_dim = is_genvec<T> && (std::decay_t<T>::dim == N);

template<typename T>
concept is_genvec_scalar = !
is_genvec<T>;

template<typename T, size_t N>
concept is_genvec_scalar_dim = !
is_genvec_dim<T, N>;

void
genvec_map() = delete;

void
genvec_for_each() = delete;

template<typename T, size_t N>
struct genvec : private std::array<T, N>
{
    using Base = std::array<T, N>;

    static inline constexpr size_t dim = N;
    using component_type = T;

    using buffer = T[N];

    using Base::operator[];
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
    constexpr genvec(genvec &&) = default;
    genvec &operator=(const genvec &) = default;
    genvec &operator=(genvec &&) noexcept(
      std::is_nothrow_move_assignable_v<T>) = default;

    template<typename... Args>
        requires(N > 0 && sizeof...(Args) == N &&
                 ((std::same_as<std::decay_t<Args>, T> && ...)))
    constexpr explicit(N == 1) genvec(Args &&...args)
      : Base{ std::forward<Args>(args)... }
    {}

    template<typename... Args>
        requires(sizeof...(Args) == N)
    constexpr explicit(N == 0) genvec(std::in_place_t, Args &&...args)
      : Base{ std::forward<Args>(args)... }
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

    template<is_genvec_dim<N> U, typename F>
    constexpr auto map(U &&rhs, F &&f) const
    {
        return genvec_map(std::forward<F>(f), *this, std::forward<U>(rhs));
    }

    template<is_genvec_dim<N> U, typename F>
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
        genvec<bool, dim> shape{};
        return shape.map([&](auto) { return x; });
    }

    template<typename U>
    static constexpr genvec convert(const genvec<U, N> &v)
    {
        return v.map([](auto &x) { return T(x); });
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
          std::forward<decltype(lhs)>(lhs),
          std::forward<decltype(rhs)>(rhs));
    }
};

template<class T, class... U>
genvec(T, U...) -> genvec<T, 1 + sizeof...(U)>;

#undef DEF_GENVEC_OP
#undef DEF_GENVEC_MUT_OP
#undef DEF_GENVEC_OP_BOTH
#undef DEF_GENVEC_SCALAR_OP
#undef DEF_GENVEC_SCALAR_MUT_OP
#undef DEF_GENVEC_SCALAR_OP_BOTH

template<typename F, is_genvec V, is_genvec_dim<std::decay_t<V>::dim>... Vs>
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

template<typename F, is_genvec V, is_genvec_dim<std::decay_t<V>::dim>... Vs>
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
    requires(is_genvec_scalar_dim<T, N>)
inline constexpr auto
operator*(const T &lhs, const genvec<U, N> &rhs)
{
    return rhs.map([&](const U &x) { return lhs * x; });
}

template<typename T, typename U, size_t N>
    requires(is_genvec_scalar_dim<T, N>)
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

template<typename T, typename U, size_t N>
inline constexpr auto
dot(const genvec<T, N> &lhs, const genvec<U, N> &rhs)
{
    static_assert(N > 0, "N not > 0");
    auto ret = lhs[0] * rhs[0];
    for (size_t i = 1; i < N; ++i)
        ret += lhs[i] * rhs[i];
    return ret;
}

template<typename T, typename U>
inline constexpr auto
cross(const genvec<T, 3> &lhs, const genvec<U, 3> &rhs)
{
    auto x = lhs[1] * rhs[2] - lhs[2] * rhs[1];
    auto y = lhs[2] * rhs[0] - lhs[0] * rhs[2];
    auto z = lhs[0] * rhs[1] - lhs[1] * rhs[0];
    genvec<decltype(x), 3> ret{};
    ret[0] = x;
    ret[1] = y;
    ret[2] = z;
    return ret;
}

template<typename T, typename U>
inline constexpr auto
det(const genvec<T, 2> &lhs, const genvec<U, 2> &rhs)
{
    return lhs[0] * rhs[1] - lhs[1] * rhs[0];
}

template<typename T, size_t N>
inline constexpr T
sum(const genvec<T, N> &v)
{
    T ret{};
    for (size_t i = 0; i < N; ++i)
        ret += v[i];
    return ret;
}

template<typename T, size_t N>
inline constexpr T
quadrance(const genvec<T, N> &v)
{
    return dot(v, v);
}

template<typename T, size_t N>
inline constexpr real
length(const genvec<T, N> &v)
{
    return sqrt(quadrance(v));
}

template<typename T, size_t N>
inline constexpr real
inverseLength(const genvec<T, N> &v)
{
    return rsqrt(quadrance(v));
}

template<typename T, size_t N>
inline constexpr auto
normalize(const genvec<T, N> &v)
{
    return v * inverseLength(v);
}

template<typename T, typename U, size_t N>
inline constexpr auto
distance(const genvec<T, N> &lhs, const genvec<U, N> &rhs)
{
    return length(lhs - rhs);
}

template<typename T, typename U, size_t N>
inline constexpr auto
inverseDistance(const genvec<T, N> &lhs, const genvec<U, N> &rhs)
{
    return inverseLength(lhs - rhs);
}

template<typename T, typename U, size_t N>
inline constexpr auto
distanceSq(const genvec<T, N> &lhs, const genvec<U, N> &rhs)
{
    return quadrance(lhs - rhs);
}

template<typename T, typename U, size_t N, typename V = int>
inline constexpr auto
reflect(const genvec<T, N> &a, const genvec<U, N> &n, const V &amp = 1)
{
    return a - n * (2 * amp * dot(n, a));
}

template<typename T, typename U, size_t N>
inline constexpr auto
min(const genvec<T, N> &lhs, const genvec<U, N> &rhs)
{
    return lhs.map(rhs, [](const T &x, const U &y) {
        using std::min;
        return min(x, y);
    });
}

template<typename T, typename U, size_t N>
inline constexpr auto
max(const genvec<T, N> &lhs, const genvec<U, N> &rhs)
{
    return lhs.map(rhs, [](const T &x, const U &y) {
        using std::max;
        return max(x, y);
    });
}

template<typename T, size_t N>
inline constexpr auto
abs(const genvec<T, N> &v)
{
    return v.map([](const T &x) {
        using std::abs;
        return abs(x);
    });
}

template<typename T, size_t N>
inline constexpr auto
recip(const genvec<T, N> &v)
{
    return v.map([](const T &x) { return recip(x); });
}

template<typename T, typename U, typename V, size_t N>
inline constexpr auto
lerp(const genvec<T, N> &a, const genvec<U, N> &b, const V &t)
{
    return a + (b - a) * t;
}

template<typename T, typename U, size_t N>
inline constexpr auto
directionFromTo(const genvec<T, N> &a, const genvec<U, N> &b)
{
    return normalize(b - a);
}

template<typename T, typename U, size_t N>
inline constexpr auto
cos(const genvec<T, N> &a, const genvec<U, N> &b)
{
    return dot(a, b) / (length(a) * length(b));
}

template<typename T, typename U, size_t N>
inline constexpr auto
projectAlong(const genvec<T, N> &a, const genvec<U, N> &x)
{
    return dot(a, x) / quadrance(x) * x;
}

template<typename T, size_t N, typename U>
inline constexpr bool
equal(const genvec<T, N> &a, const genvec<T, N> &b, const U &epsi)
{
    for (size_t i = 0; i < N; ++i)
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
