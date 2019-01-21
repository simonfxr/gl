#ifndef GL_MATH_GENMAT_HPP
#define GL_MATH_GENMAT_HPP

#include "math/genvec.hpp"
#include "math/math.hpp"
#include "math/real.hpp"

#include <array>

#include <type_traits>

#define DEF_GENMAT_OP(op)                                                      \
    template<typename U>                                                       \
    constexpr auto operator op(const genmat<U, N> &rhs) const                  \
    {                                                                          \
        using V = std::decay_t<decltype(columns[0][0] op rhs[0][0])>;          \
        genmat<V, N> ret{};                                                    \
        for (size_t i = 0; i < N; ++i)                                         \
            for (size_t j = 0; j < N; ++j)                                     \
                ret[i][j] = columns[i][j] op rhs[i][j];                        \
        return ret;                                                            \
    }

#define DEF_GENMAT_SCALAR_OP(op)                                               \
    template<typename U,                                                       \
             typename = std::enable_if_t<!is_genmat_v<U> && !is_genvec_v<U>>>  \
    constexpr auto operator op(const U &rhs) const                             \
    {                                                                          \
        using V = std::decay_t<decltype(columns[0][0] op rhs)>;                \
        genmat<V, N> ret{};                                                    \
        for (size_t i = 0; i < N; ++i)                                         \
            for (size_t j = 0; j < N; ++j)                                     \
                ret[i][j] = columns[i][j] op rhs;                              \
        return ret;                                                            \
    }

#define DEF_GENMAT_MUT_OP(op)                                                  \
    template<typename U>                                                       \
    constexpr genmat &operator PP_CAT(op, =)(const genmat<U, N> &rhs)          \
    {                                                                          \
        return *this = *this op rhs;                                           \
    }

#define DEF_GENMAT_SCALAR_MUT_OP(op)                                           \
    template<typename U,                                                       \
             typename = std::enable_if_t<!is_genmat_v<U> && !is_genvec_v<U>>>  \
    constexpr genmat &operator PP_CAT(op, =)(const U &rhs)                     \
    {                                                                          \
        return *this = *this op rhs;                                           \
    }

#define DEF_GENMAT_OP_BOTH(op)                                                 \
    DEF_GENMAT_OP(op)                                                          \
    DEF_GENMAT_MUT_OP(op)

#define DEF_GENMAT_SCALAR_OP_BOTH(op)                                          \
    DEF_GENMAT_SCALAR_OP(op)                                                   \
    DEF_GENMAT_SCALAR_MUT_OP(op)

namespace math {

template<typename T, size_t N>
struct genmat;

template<typename T>
struct is_genmat : public std::false_type
{};

template<typename T, size_t N>
struct is_genmat<genmat<T, N>> : public std::true_type
{};

template<typename T>
inline constexpr bool is_genmat_v = is_genmat<T>::value;

template<typename T, size_t N>
struct genmat
{
    static inline constexpr size_t column_size = N;
    using column_type = genvec<T, N>;
    using component_type = T;
    static inline constexpr size_t size = N * N;
    static inline constexpr size_t padded_size = N * column_type::padded_size;
    using buffer = T[size];

    column_type columns[N];

    HU_FORCE_INLINE constexpr column_type &operator[](size_t i)
    {
        return columns[i];
    }

    HU_FORCE_INLINE constexpr const column_type &operator[](size_t i) const
    {
        return columns[i];
    }

    HU_FORCE_INLINE constexpr inline const T &operator()(size_t i,
                                                         size_t j) const
    {
        return columns[i][j];
    }

    HU_FORCE_INLINE constexpr inline T &operator()(size_t i, size_t j)
    {
        return columns[i][j];
    }

    template<typename F>
    constexpr auto map(F &&f) const
    {
        using U = std::decay_t<std::invoke_result_t<F, const column_type &>>;
        genmat<U, N> ret{};
        for (size_t i = 0; i < N; ++i)
            ret[i] = f(columns[i]);
        return ret;
    }

    template<typename U, typename F>
    constexpr auto map(const genmat<U, N> &rhs, F &&f) const
    {
        using V = std::decay_t<
          std::invoke_result_t<F, const column_type &, const column_type &>>;
        genmat<V, N> ret{};
        for (size_t i = 0; i < N; ++i)
            ret[i] = f(columns[i], rhs[i]);
        return ret;
    }

    constexpr genmat operator-() const
    {
        return map([](const T &x) { return -x; });
    }

    DEF_GENMAT_OP_BOTH(+)
    DEF_GENMAT_OP_BOTH(-)

    DEF_GENMAT_SCALAR_OP_BOTH(/)
    DEF_GENMAT_SCALAR_OP_BOTH(*)

    static constexpr genmat fill(const T &x)
    {
        genmat ret{};
        for (size_t i = 0; i < N; ++i)
            for (size_t j = 0; j < N; ++j)
                ret[i][j] = x;
        return ret;
    }

    template<typename... Args>
    static constexpr genmat make(Args &&... args)
    {
        static_assert(sizeof...(args) == N,
                      "can only initialize using exactly N elements");
        return { std::forward<Args>(args)... };
    }

    static constexpr genmat identity()
    {
        genmat A{};
        for (size_t i = 0; i < N; ++i)
            A[i][i] = T(1);
        return A;
    }

    static constexpr genmat load(const buffer b)
    {
        genmat ret{};
        for (size_t i = 0; i < N; ++i)
            for (size_t j = 0; j < N; ++j)
                ret[i][j] = b[i * N + j];
        return ret;
    }

    template<typename U>
    static constexpr genmat convert(const genmat<U, N> &A)
    {
        genmat ret{};
        for (size_t i = 0; i < N; ++i)
            for (size_t j = 0; j < N; ++j)
                ret[i][j] = T(A[i][j]);
        return ret;
    }

    const T *data() const { return columns[0].data(); }

    T *data() { return columns[0].data(); }

    const column_type *begin() const { return columns; }
    column_type *begin() { return columns; }

    const column_type *end() const { return columns + N; }
    column_type *end() { return columns + N; }

    const column_type *cbegin() const { return columns; }

    const column_type *cend() const { return columns + N; }
};

#undef DEF_GENMAT_OP
#undef DEF_GENMAT_MUT_OP
#undef DEF_GENMAT_OP_BOTH
#undef DEF_GENMAT_SCALAR_OP
#undef DEF_GENMAT_SCALAR_MUT_OP
#undef DEF_GENMAT_SCALAR_OP_BOTH

template<typename T, size_t N>
inline constexpr void
load(T *buffer, const genmat<T, N> &v)
{
    for (size_t i = 0; i < N; ++i)
        for (size_t j = 0; j < N; ++j)
            buffer[i * N + j] = v[i][j];
}

template<typename T,
         typename U,
         size_t N,
         typename = std::enable_if_t<!is_genmat_v<U> && !is_genvec_v<U>>>
inline constexpr auto operator*(const T &lhs, const genmat<U, N> &rhs)
{
    using V = std::decay_t<decltype(lhs * rhs[0][0])>;
    genmat<V, N> ret{};
    for (size_t i = 0; i < N; ++i)
        for (size_t j = 0; j < N; ++j)
            ret[i][j] = lhs * rhs[i][j];
    return ret;
}

template<typename T, typename U, size_t N>
inline constexpr auto operator*(const genmat<T, N> &A, const genvec<U, N> &v)
{
    using V = decltype(A[0][0] * v[0]);
    genvec<V, N> u{};
    for (size_t j = 0; j < N; ++j)
        for (size_t i = 0; i < N; ++i)
            u[i] += A[j][i] * v[j];
    return u;
}

template<typename T, typename U, size_t N>
inline constexpr auto
transform(const genmat<T, N> &A, const genvec<U, N> &v)
{
    return A * v;
}

template<typename T, typename U, size_t N>
HU_FORCE_INLINE inline constexpr auto
inline_mat_mul(const genmat<T, N> &A, const genmat<U, N> &B)
{
    using V = decltype(A[0][0] * B[0][0]);
    genmat<V, N> C{};
    for (size_t j = 0; j < N; ++j)
        for (size_t i = 0; i < N; ++i)
            for (size_t k = 0; k < N; ++k)
                C[j][i] += A[k][i] * B[j][k];
    return C;
}

template<typename T, typename U, size_t N>
inline constexpr auto operator*(const genmat<T, N> &A, const genmat<U, N> &B)
  -> genmat<decltype(A[0][0] * B[0][0]), N>
{
    return inline_mat_mul(A, B);
}

extern template MATH_API genmat<float, 3> operator*(const genmat<float, 3> &,
                                                    const genmat<float, 3> &);

extern template MATH_API genmat<double, 3> operator*(const genmat<double, 3> &,
                                                     const genmat<double, 3> &);

extern template MATH_API genmat<float, 4> operator*(const genmat<float, 4> &,
                                                    const genmat<float, 4> &);

extern template MATH_API genmat<double, 4> operator*(const genmat<double, 4> &,
                                                     const genmat<double, 4> &);

template<typename T, typename U, size_t N>
inline constexpr genmat<T, N> &
operator*=(genmat<T, N> &A, const genmat<U, N> &B)
{
    return A = inline_mat_mul(A, B);
}

extern template MATH_API genmat<float, 3> &
operator*=(genmat<float, 3> &, const genmat<float, 3> &);

extern template MATH_API genmat<double, 3> &
operator*=(genmat<double, 3> &, const genmat<double, 3> &);

extern template MATH_API genmat<float, 4> &
operator*=(genmat<float, 4> &, const genmat<float, 4> &);

extern template MATH_API genmat<double, 4> &
operator*=(genmat<double, 4> &, const genmat<double, 4> &);

template<typename T, typename U, size_t N>
inline constexpr bool
operator==(const genmat<T, N> &lhs, const genmat<U, N> &rhs)
{
    for (size_t i = 0; i < N; ++i)
        for (size_t j = 0; j < N; ++j)
            if (lhs[i][j] != rhs[i][j])
                return false;
    return true;
}

template<typename T, typename U, size_t N>
inline constexpr bool
operator!=(const genmat<T, N> &lhs, const genmat<U, N> &rhs)
{
    return !(lhs == rhs);
}

template<typename T, size_t N>
inline constexpr genvec<T, N>
sum(const genmat<T, N> &v)
{
    genvec<T, N> ret{};
    for (size_t i = 0; i < N; ++i)
        ret += v[i];
    return ret;
}

template<typename T, typename U, size_t N>
inline constexpr auto
min(const genmat<T, N> &lhs, const genmat<U, N> &rhs)
{
    return lhs.map(rhs, [](const T &x, const U &y) {
        using std::min;
        return min(x, y);
    });
}

template<typename T, typename U, size_t N>
inline constexpr auto
max(const genmat<T, N> &lhs, const genmat<U, N> &rhs)
{
    return lhs.map(rhs, [](const T &x, const U &y) {
        using std::max;
        return max(x, y);
    });
}

template<typename T, size_t N>
inline constexpr auto
abs(const genmat<T, N> &v)
{
    return v.map([](const T &x) {
        using std::abs;
        return abs(x);
    });
}

template<typename T, typename U, typename V, size_t N>
inline constexpr auto
lerp(const genmat<T, N> &a, const genmat<U, N> &b, const V &t)
{
    return a + (b - a) * t;
}

template<typename T, size_t N, typename U>
inline constexpr bool
equal(const genmat<T, N> &a, const genmat<T, N> &b, const U &epsi)
{
    for (size_t i = 0; i < N; ++i)
        if (distance(a[i], b[i]) > epsi)
            return false;
    return true;
}

template<typename T, size_t N>
inline constexpr genmat<T, N>
transpose(const genmat<T, N> &A)
{
    genmat<T, N> B{};
    for (size_t i = 0; i < N; ++i)
        for (size_t j = 0; j < N; ++j)
            B[j][i] = A[i][j];
    return B;
}

template<typename T>
inline constexpr T
determinant(const genmat<T, 2> &A)
{
    return A[0][0] * A[1][1] - A[0][1] * A[1][0];
}

template<typename T>
inline constexpr T
determinant(const genmat<T, 3> &A)
{
    auto d = A[0][0] * (A[1][1] * A[2][2] - A[1][2] * A[2][1]);
    d -= A[0][1] * (A[1][0] * A[2][2] - A[1][2] * A[2][0]);
    d += A[0][2] * (A[1][0] * A[2][1] - A[1][1] * A[2][0]);
    return d;
}

template<typename T>
inline constexpr T
determinant(const genmat<T, 4> &A)
{
    auto d = A[0][0] * (A[1][1] * (A[2][2] * A[3][3] - A[2][3] * A[3][2]) -
                        A[1][2] * (A[2][1] * A[3][3] - A[2][3] * A[3][1]) +
                        A[1][3] * (A[2][1] * A[3][2] - A[2][2] * A[3][1]));
    d -= A[0][1] * (A[1][0] * (A[2][2] * A[3][3] - A[2][3] * A[3][2]) -
                    A[1][2] * (A[2][0] * A[3][3] - A[2][3] * A[3][0]) +
                    A[1][3] * (A[2][0] * A[3][2] - A[2][2] * A[3][0]));
    d += A[0][2] * (A[1][0] * (A[2][1] * A[3][3] - A[2][3] * A[3][1]) -
                    A[1][1] * (A[2][0] * A[3][3] - A[2][3] * A[3][0]) +
                    A[1][3] * (A[2][0] * A[3][1] - A[2][1] * A[3][0]));
    d -= A[0][3] * (A[1][0] * (A[2][1] * A[3][2] - A[2][2] * A[3][1]) -
                    A[1][1] * (A[2][0] * A[3][2] - A[2][2] * A[3][0]) +
                    A[1][2] * (A[2][0] * A[3][1] - A[2][1] * A[3][0]));
    return d;
}

template<typename T>
inline constexpr genmat<T, 2>
inverse(const genmat<T, 2> &A)
{
    genmat<T, 2> B{};
    B[0][0] = A[1][1];
    B[0][1] = -A[1][0];
    B[1][0] = -A[0][1];
    B[1][1] = A[0][0];
    return B / determinant(A);
}

template<typename T>
inline constexpr genmat<T, 3>
inverse(const genmat<T, 3> &A)
{
    const auto AT = transpose(A);
    struct
    {
        typename genmat<T, 3>::buffer a;
    } m{};
    load(m.a, AT);

    auto t4 = m.a[0] * m.a[4];
    auto t6 = m.a[0] * m.a[5];
    auto t8 = m.a[1] * m.a[3];
    auto t10 = m.a[2] * m.a[3];
    auto t12 = m.a[1] * m.a[6];
    auto t14 = m.a[2] * m.a[6];
    // determinant
    auto t16 = (t4 * m.a[8] - t6 * m.a[7] - t8 * m.a[8] + t10 * m.a[7] +
                t12 * m.a[5] - t14 * m.a[4]);

    if (t16 == 0)
        return {};
    auto t17 = 1 / t16;

    typename genmat<T, 3>::buffer a{};

    a[0] = (m.a[4] * m.a[8] - m.a[5] * m.a[7]) * t17;
    a[1] = -(m.a[1] * m.a[8] - m.a[2] * m.a[7]) * t17;
    a[2] = (m.a[1] * m.a[5] - m.a[2] * m.a[4]) * t17;
    a[3] = -(m.a[3] * m.a[8] - m.a[5] * m.a[6]) * t17;
    a[4] = (m.a[0] * m.a[8] - t14) * t17;
    a[5] = -(t6 - t10) * t17;
    a[6] = (m.a[3] * m.a[7] - m.a[4] * m.a[6]) * t17;
    a[7] = -(m.a[0] * m.a[7] - t12) * t17;
    a[8] = (t4 - t8) * t17;

    return transpose(genmat<T, 3>::load(a));
}

extern template MATH_API genmat<float, 3>
inverse(const genmat<float, 3> &A);

extern template MATH_API genmat<double, 3>
inverse(const genmat<double, 3> &);

template<typename T>
inline constexpr genmat<T, 4>
inverse(const genmat<T, 4> &A)
{
    auto m = &A[0][0];
    genmat<T, 4> Ainv{};
    auto inv = &Ainv[0][0];

    // cramers rule on the transposed matrix, dear reader have trust :-)
    inv[0] = m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - m[9] * m[6] * m[15] +
             m[9] * m[7] * m[14] + m[13] * m[6] * m[11] - m[13] * m[7] * m[10];
    inv[4] = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] +
             m[8] * m[6] * m[15] - m[8] * m[7] * m[14] - m[12] * m[6] * m[11] +
             m[12] * m[7] * m[10];
    inv[8] = m[4] * m[9] * m[15] - m[4] * m[11] * m[13] - m[8] * m[5] * m[15] +
             m[8] * m[7] * m[13] + m[12] * m[5] * m[11] - m[12] * m[7] * m[9];
    inv[12] = -m[4] * m[9] * m[14] + m[4] * m[10] * m[13] +
              m[8] * m[5] * m[14] - m[8] * m[6] * m[13] - m[12] * m[5] * m[10] +
              m[12] * m[6] * m[9];
    inv[1] = -m[1] * m[10] * m[15] + m[1] * m[11] * m[14] +
             m[9] * m[2] * m[15] - m[9] * m[3] * m[14] - m[13] * m[2] * m[11] +
             m[13] * m[3] * m[10];
    inv[5] = m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - m[8] * m[2] * m[15] +
             m[8] * m[3] * m[14] + m[12] * m[2] * m[11] - m[12] * m[3] * m[10];
    inv[9] = -m[0] * m[9] * m[15] + m[0] * m[11] * m[13] + m[8] * m[1] * m[15] -
             m[8] * m[3] * m[13] - m[12] * m[1] * m[11] + m[12] * m[3] * m[9];
    inv[13] = m[0] * m[9] * m[14] - m[0] * m[10] * m[13] - m[8] * m[1] * m[14] +
              m[8] * m[2] * m[13] + m[12] * m[1] * m[10] - m[12] * m[2] * m[9];
    inv[2] = m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - m[5] * m[2] * m[15] +
             m[5] * m[3] * m[14] + m[13] * m[2] * m[7] - m[13] * m[3] * m[6];
    inv[6] = -m[0] * m[6] * m[15] + m[0] * m[7] * m[14] + m[4] * m[2] * m[15] -
             m[4] * m[3] * m[14] - m[12] * m[2] * m[7] + m[12] * m[3] * m[6];
    inv[10] = m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[4] * m[1] * m[15] +
              m[4] * m[3] * m[13] + m[12] * m[1] * m[7] - m[12] * m[3] * m[5];
    inv[14] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13] + m[4] * m[1] * m[14] -
              m[4] * m[2] * m[13] - m[12] * m[1] * m[6] + m[12] * m[2] * m[5];
    inv[3] = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11] -
             m[5] * m[3] * m[10] - m[9] * m[2] * m[7] + m[9] * m[3] * m[6];
    inv[7] = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11] +
             m[4] * m[3] * m[10] + m[8] * m[2] * m[7] - m[8] * m[3] * m[6];
    inv[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11] -
              m[4] * m[3] * m[9] - m[8] * m[1] * m[7] + m[8] * m[3] * m[5];
    inv[15] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10] +
              m[4] * m[2] * m[9] + m[8] * m[1] * m[6] - m[8] * m[2] * m[5];

    auto det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
    if (det == 0)
        return {};
    return Ainv * recip(det);
}

extern template MATH_API genmat<float, 4>
inverse(const genmat<float, 4> &A);

extern template MATH_API genmat<double, 4>
inverse(const genmat<double, 4> &A);

template<typename T>
inline genmat<T, 3>
orthonormalBasis(const genmat<T, 3> &A)
{
    auto u = normalize(A[0]);
    auto v = normalize(A[1] - projectAlong(A[1], u));
    return { u, v, cross(u, v) };
}

template<typename T>
inline constexpr genmat<T, 3>
coordinateSystem(const genvec<T, 3> &a)
{
    auto aa = abs(a);
    using V = genvec<T, 3>;
    V b{};

    if (aa[0] > aa[1] && aa[0] > aa[2])
        b = V::make(-a[2], T{}, a[0]);
    else if (aa[1] > aa[2])
        b = V::make(a[1], -a[0], T{});
    else
        b = V::make(T{}, a[2], -a[1]);
    return mat3(a, b, cross(a, b));
}

template<typename OStream, typename T, size_t N>
inline OStream &
operator<<(OStream &out, const genmat<T, N> &A)
{
    out << "mat" << N << "[";
    bool sep = false;
    for (size_t i = 0; i < N; ++i) {
        if (sep)
            out << "; ";
        sep = true;
        bool esep = false;
        for (size_t j = 0; j < N; ++j) {
            if (esep)
                out << " ";
            esep = true;
            out << A[j][i];
        }
    }
    return out << "]";
}

} // namespace math

BEGIN_NO_WARN_MISMATCHED_TAGS
namespace std {
template<typename T, size_t N>
struct tuple_size<math::genmat<T, N>> : public std::integral_constant<size_t, N>
{};
END_NO_WARN_MISMATCHED_TAGS

template<size_t I, typename T, size_t N>
HU_FORCE_INLINE inline const math::genvec<T, N> &
get(const math::genmat<T, N> &A)
{
    static_assert(I < N);
    return A[I];
}

template<size_t I, typename T, size_t N>
HU_FORCE_INLINE inline math::genvec<T, N> &
get(math::genmat<T, N> &A)
{
    static_assert(I < N);
    return A[I];
}
} // namespace std

#endif
