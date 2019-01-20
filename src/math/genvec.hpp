#ifndef GL_MATH_GENVEC_HPP
#define GL_MATH_GENVEC_HPP

#include "math/real.hpp"
#include <array>

#include <type_traits>

#define DEF_GENVEC_OP(op)                                                      \
    template<typename U>                                                       \
    constexpr auto operator op(const genvec<U, N> &rhs) const                  \
    {                                                                          \
        using V = std::decay_t<decltype(components[0] op rhs[0])>;             \
        genvec<V, N> ret{};                                                    \
        for (size_t i = 0; i < N; ++i)                                         \
            ret[i] = components[i] op rhs[i];                                  \
        return ret;                                                            \
    }

#define DEF_GENVEC_SCALAR_OP(op)                                               \
    template<typename U, typename = std::enable_if_t<!is_genvec_v<U>>>         \
    constexpr auto operator op(const U &rhs) const                             \
    {                                                                          \
        using V = std::decay_t<decltype(components[0] op rhs)>;                \
        genvec<V, N> ret{};                                                    \
        for (size_t i = 0; i < N; ++i)                                         \
            ret[i] = components[i] op rhs;                                     \
        return ret;                                                            \
    }

#define DEF_GENVEC_MUT_OP(op)                                                  \
    template<typename U>                                                       \
    constexpr genvec &operator PP_CAT(op, =)(const genvec<U, N> &rhs)          \
    {                                                                          \
        return *this = *this op rhs;                                           \
    }

#define DEF_GENVEC_SCALAR_MUT_OP(op)                                           \
    template<typename U, typename = std::enable_if_t<!is_genvec_v<U>>>         \
    constexpr genvec &operator PP_CAT(op, =)(const U &rhs)                     \
    {                                                                          \
        return *this = *this op rhs;                                           \
    }

#define DEF_GENVEC_OP_BOTH(op)                                                 \
    DEF_GENVEC_OP(op)                                                          \
    DEF_GENVEC_MUT_OP(op)

#define DEF_GENVEC_SCALAR_OP_BOTH(op)                                          \
    DEF_GENVEC_SCALAR_OP(op)                                                   \
    DEF_GENVEC_SCALAR_MUT_OP(op)

namespace math {

template<typename T, size_t N>
struct gltype_mapping;

template<typename T, size_t N>
struct genvec;

template<typename T>
struct is_genvec : public std::false_type
{};

template<typename T, size_t N>
struct is_genvec<genvec<T, N>> : public std::true_type
{};

template<typename T>
inline constexpr bool is_genvec_v = is_genvec<T>::value;

template<typename T, size_t N>
struct genvec
{
    static const size_t size = N;
    static const size_t padded_size = N;
    using component_type = T;
    using buffer = T[N];
    using gl = typename gltype_mapping<T, N>::type;

    T components[N];

    constexpr T &operator[](size_t i) { return components[i]; }

    constexpr const T &operator[](size_t i) const { return components[i]; }

    template<typename F>
    constexpr auto map(F &&f) const
    {
        using U = std::decay_t<std::invoke_result_t<F, const T &>>;
        genvec<U, N> ret{};
        for (size_t i = 0; i < N; ++i)
            ret[i] = f(components[i]);
        return ret;
    }

    template<typename U, typename F>
    constexpr auto map(const genvec<U, N> &rhs, F &&f) const
    {
        using V = std::decay_t<std::invoke_result_t<F, const T &, const U &>>;
        genvec<V, N> ret{};
        for (size_t i = 0; i < N; ++i)
            ret[i] = f(components[i], rhs[i]);
        return ret;
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

    static constexpr genvec fill(const T &x)
    {
        genvec ret{};
        for (size_t i = 0; i < N; ++i)
            ret[i] = x;
        return ret;
    }

    template<typename... Args>
    static constexpr genvec make(Args &&... args)
    {
        static_assert(sizeof...(args) == N,
                      "can only initialize using exactly N elements");
        return { T{ std::forward<Args>(args) }... };
    }

    static constexpr genvec load(const buffer b)
    {
        genvec ret{};
        for (size_t i = 0; i < N; ++i)
            ret[i] = b[i];
        return ret;
    }

    template<typename U>
    static constexpr genvec convert(const genvec<U, N> &v)
    {
        genvec ret{};
        for (size_t i = 0; i < N; ++i)
            ret[i] = T(v[i]);
        return ret;
    }

    const buffer &data() const { return components; }
    buffer &data() { return components; }

    const T *begin() const { return data(); }
    T *begin() { return data(); }

    const T *end() const { return components + N; }
    T *end() { return components + N; }

    const T *cbegin() const { return begin(); }
    const T *cend() const { return end(); }
};

#undef DEF_GENVEC_OP
#undef DEF_GENVEC_MUT_OP
#undef DEF_GENVEC_OP_BOTH
#undef DEF_GENVEC_SCALAR_OP
#undef DEF_GENVEC_SCALAR_MUT_OP
#undef DEF_GENVEC_SCALAR_OP_BOTH

template<typename T, size_t N>
struct gltype_mapping
{
    using type = void;
};

template<typename T, size_t N>
constexpr void
load(T *buffer, const genvec<T, N> &v)
{
    for (size_t i = 0; i < N; ++i)
        buffer[i] = v[i];
}

template<typename T, typename U, size_t N>
constexpr auto operator*(const T &lhs, const genvec<U, N> &rhs)
{
    return rhs.map([&](const U &x) { return lhs * x; });
}

template<typename T, typename U, size_t N>
constexpr auto
operator/(const T &lhs, const genvec<U, N> &rhs)
{
    return rhs.map([&](const U &x) { return lhs / x; });
}

template<typename T, typename U, size_t N>
constexpr bool
operator==(const genvec<T, N> &lhs, const genvec<U, N> &rhs)
{
    for (size_t i = 0; i < N; ++i)
        if (lhs[i] != rhs[i])
            return false;
    return true;
}

template<typename T, typename U, size_t N>
constexpr bool
operator!=(const genvec<T, N> &lhs, const genvec<U, N> &rhs)
{
    return !(lhs == rhs);
}

template<typename T, typename U, size_t N>
constexpr auto
dot(const genvec<T, N> &lhs, const genvec<U, N> &rhs)
{
    static_assert(N > 0, "N not > 0");
    auto ret = lhs[0] * rhs[0];
    for (size_t i = 1; i < N; ++i)
        ret += lhs[i] * rhs[i];
    return ret;
}

template<typename T, typename U>
constexpr auto
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

template<typename T, size_t N>
constexpr T
sum(const genvec<T, N> &v)
{
    T ret{};
    for (size_t i = 0; i < N; ++i)
        ret += v[i];
    return ret;
}

template<typename T, size_t N>
constexpr T
quadrance(const genvec<T, N> &v)
{
    return dot(v, v);
}

template<typename T, size_t N>
constexpr real
length(const genvec<T, N> &v)
{
    return sqrt(quadrance(v));
}

template<typename T, size_t N>
constexpr real
inverseLength(const genvec<T, N> &v)
{
    return rsqrt(quadrance(v));
}

template<typename T, size_t N>
constexpr auto
normalize(const genvec<T, N> &v)
{
    return v * inverseLength(v);
}

template<typename T, typename U, size_t N>
constexpr auto
distance(const genvec<T, N> &lhs, const genvec<U, N> &rhs)
{
    return length(lhs - rhs);
}

template<typename T, typename U, size_t N>
constexpr auto
inverseDistance(const genvec<T, N> &lhs, const genvec<U, N> &rhs)
{
    return inverseLength(lhs - rhs);
}

template<typename T, typename U, size_t N>
constexpr auto
distanceSq(const genvec<T, N> &lhs, const genvec<U, N> &rhs)
{
    return quadrance(lhs - rhs);
}

template<typename T, typename U, size_t N, typename V = int>
constexpr auto
reflect(const genvec<T, N> &a, const genvec<U, N> &n, const V &amp = 1)
{
    return a - n * (2 * amp * dot(n, a));
}

template<typename T, typename U, size_t N>
constexpr auto
min(const genvec<T, N> &lhs, const genvec<U, N> &rhs)
{
    return lhs.map(rhs, [](const T &x, const U &y) {
        using std::min;
        return min(x, y);
    });
}

template<typename T, typename U, size_t N>
constexpr auto
max(const genvec<T, N> &lhs, const genvec<U, N> &rhs)
{
    return lhs.map(rhs, [](const T &x, const U &y) {
        using std::max;
        return max(x, y);
    });
}

template<typename T, size_t N>
constexpr auto
abs(const genvec<T, N> &v)
{
    return v.map([](const T &x) {
        using std::abs;
        return abs(x);
    });
}

template<typename T, size_t N>
constexpr auto
recip(const genvec<T, N> &v)
{
    return v.map([](const T &x) { return recip(x); });
}

template<typename T, typename U, typename V, size_t N>
constexpr auto
lerp(const genvec<T, N> &a, const genvec<U, N> &b, const V &t)
{
    return a + (b - a) * t;
}

template<typename T, typename U, size_t N>
constexpr auto
directionFromTo(const genvec<T, N> &a, const genvec<U, N> &b)
{
    return normalize(b - a);
}

template<typename T, typename U, size_t N>
constexpr auto
cos(const genvec<T, N> &a, const genvec<U, N> &b)
{
    return dot(a, b) / (length(a) * length(b));
}

template<typename T, typename U, size_t N>
constexpr auto
projectAlong(const genvec<T, N> &a, const genvec<U, N> &x)
{
    return dot(a, x) / quadrance(x) * x;
}

template<typename T, size_t N, typename U>
constexpr bool
equal(const genvec<T, N> &a, const genvec<T, N> &b, const U &epsi)
{
    for (size_t i = 0; i < N; ++i)
        if (distance(a[i], b[i]) > epsi)
            return false;
    return true;
}

template<typename OStream, typename T, size_t N>
OStream &
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

} // namespace math

BEGIN_NO_WARN_MISMATCHED_TAGS
namespace std {
template<typename T, size_t N>
struct tuple_size<math::genvec<T, N>> : public std::integral_constant<size_t, N>
{};
END_NO_WARN_MISMATCHED_TAGS

template<size_t I, typename T, size_t N>
const T &
get(const math::genvec<T, N> &v)
{
    static_assert(I < N);
    return v[I];
}

template<size_t I, typename T, size_t N>
T &
get(math::genvec<T, N> &v)
{
    static_assert(I < N);
    return v[I];
}

} // namespace std

#endif
