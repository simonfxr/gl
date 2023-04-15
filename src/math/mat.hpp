#ifndef GL_MATH_GENMAT_HPP
#define GL_MATH_GENMAT_HPP

#include "math/math.hpp"
#include "math/real.hpp"
#include "math/vec.hpp"

#include <array>

#include <type_traits>

namespace math {

template<typename T, size_t N>
struct genmat;

struct in_place_columns_t
{};

void
mat_map() = delete;

template<typename T, size_t N>
struct genmat : private vec<vec<T, N>, N>
{
    static inline constexpr size_t column_size = N;
    using column_type = vec<T, N>;
    using scalar_type = T;
    static inline constexpr size_t dim = N;
    using buffer = T[N * N];

    using Base = vec<vec<T, N>, N>;
    using Self = genmat<T, N>;

    using vec<vec<T, N>, N>::vec;
    using Base::operator[];
    using Base::back;
    using Base::begin;
    using Base::cbegin;
    using Base::cend;
    using Base::crbegin;
    using Base::crend;
    using Base::empty;
    using Base::end;
    using Base::fill;
    using Base::front;
    using Base::rbegin;
    using Base::rend;
    using Base::size;
    using Base::operator std::span<column_type, N>;

    constexpr genmat() = default;
    constexpr genmat(const genmat &) = default;
    constexpr genmat(genmat &&) noexcept(
      std::is_nothrow_move_constructible_v<Base>) = default;
    genmat &operator=(const genmat &) = default;
    genmat &operator=(genmat &&) noexcept(
      std::is_nothrow_move_assignable_v<Base>) = default;

    constexpr genmat(
      in_place_columns_t,
      std::same_as<Base> auto
        &&val) noexcept(std::is_nothrow_constructible_v<Base, decltype(val)>)
      : Base{ bl_fwd(val) }
    {}

    HU_FORCE_INLINE constexpr inline const T &operator()(size_t i,
                                                         size_t j) const
    {
        return operator[](i)[j];
    }

    HU_FORCE_INLINE constexpr inline T &operator()(size_t i, size_t j)
    {
        return operator[](i)[j];
    }

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
    MATH_DEF_TENSOR_SCALAR_OP_BOTH(*)
    MATH_DEF_TENSOR_SCALAR_OP_BOTH(/)

    MATH_DEF_TENSOR_OP_BOTH(&, requires(std::integral<T>))
    MATH_DEF_TENSOR_OP_BOTH(|, requires(std::integral<T>))
    MATH_DEF_TENSOR_OP_BOTH(^, requires(std::integral<T>))

    constexpr T *data() noexcept { return Base::data()[0].data(); }
    constexpr const T *data() const noexcept { return Base::data()[0].data(); }

    constexpr operator std::span<T, N * N>() noexcept { return { begin(), N }; }

    constexpr operator std::span<const T, N * N>() const noexcept
    {
        return { begin(), N * N };
    }

    static constexpr genmat broadcast(const T &x)
    {
        genmat ret{};
        for (size_t i = 0; i < N; ++i)
            for (size_t j = 0; j < N; ++j)
                ret[i][j] = x;
        return ret;
    }

    template<typename... Args>
    static constexpr genmat make(Args &&...args)
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

    template<typename U>
    static constexpr genmat convert(const genmat<U, N> &A)
    {
        genmat ret{};
        for (size_t i = 0; i < N; ++i)
            for (size_t j = 0; j < N; ++j)
                ret[i][j] = T(A[i][j]);
        return ret;
    }

    constexpr auto operator()(vec_of_dim<N> auto &&rhs) const
    {
        return transform(*this, rhs);
    }

    constexpr Base &&columns() && { return static_cast<Base &&>(*this); }

    constexpr Base &columns() & { return static_cast<Base &>(*this); }

    constexpr const Base &&columns() const &&
    {
        return static_cast<const Base &&>(*this);
    }

    constexpr const Base &columns() const &
    {
        return static_cast<const Base &>(*this);
    }

    friend constexpr auto invoke(bl::map_f,
                                 auto &&f,
                                 same_tensor<Self> auto &&m0,
                                 same_shape<Self> auto &&...ms)
    {
        return genmat{
            in_place_columns_t{},
            bl::map([&](auto &&...cols) { return bl::map(f, bl_fwd(cols)...); },
                    bl_fwd(m0).columns(),
                    bl_fwd(ms).columns()...)
        };
    }

    friend constexpr void invoke(bl::for_each_f,
                                 auto &&f,
                                 same_tensor<Self> auto &&m0,
                                 same_shape<Self> auto &&...ms)
    {
        bl::for_each(
          [&](auto &&...cols) { return bl::for_each(f, bl_fwd(cols)...); },
          bl_fwd(m0).columns(),
          bl_fwd(ms).columns()...);
    }

    friend constexpr auto operator*(same_tensor<Self> auto &&A,
                                    same_shape<Self> auto &&B)
    {
        using V = std::decay_t<decltype(A[0][0] * B[0][0])>;
        genmat<V, N> C{};
        for (size_t j = 0; j < N; ++j)
            for (size_t i = 0; i < N; ++i)
                for (size_t k = 0; k < N; ++k)
                    C[j][i] += A[k][i] * B[j][k];
        return C;
    }
};

template<a_vec T>
genmat(in_place_columns_t, T) -> genmat<tensor_scalar_type<T>, tensor_dim<T>>;

template<typename T, typename U, size_t N>
inline constexpr auto
transform(const genmat<T, N> &A, const vec<U, N> &v)
{
    using V = std::decay_t<decltype(A[0][0] * v[0])>;
    vec<V, N> u{};
    for (size_t j = 0; j < N; ++j)
        for (size_t i = 0; i < N; ++i)
            u[i] += A[j][i] * v[j];
    return u;
}

template<typename T, typename U, size_t N>
inline constexpr genmat<T, N> &
operator*=(genmat<T, N> &A, const genmat<U, N> &B)
{
    return A = A * B;
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
inline constexpr vec<T, N>
sum(const genmat<T, N> &v)
{
    vec<T, N> ret{};
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
    auto mm = AT.data();

    auto t4 = mm[0] * mm[4];
    auto t6 = mm[0] * mm[5];
    auto t8 = mm[1] * mm[3];
    auto t10 = mm[2] * mm[3];
    auto t12 = mm[1] * mm[6];
    auto t14 = mm[2] * mm[6];
    // determinant
    auto t16 = (t4 * mm[8] - t6 * mm[7] - t8 * mm[8] + t10 * mm[7] +
                t12 * mm[5] - t14 * mm[4]);

    if (t16 == 0)
        return {};
    auto t17 = 1 / t16;

    genmat<T, 3> ret{};
    auto a = ret.data();

    a[0] = (mm[4] * mm[8] - mm[5] * mm[7]) * t17;
    a[1] = -(mm[1] * mm[8] - mm[2] * mm[7]) * t17;
    a[2] = (mm[1] * mm[5] - mm[2] * mm[4]) * t17;
    a[3] = -(mm[3] * mm[8] - mm[5] * mm[6]) * t17;
    a[4] = (mm[0] * mm[8] - t14) * t17;
    a[5] = -(t6 - t10) * t17;
    a[6] = (mm[3] * mm[7] - mm[4] * mm[6]) * t17;
    a[7] = -(mm[0] * mm[7] - t12) * t17;
    a[8] = (t4 - t8) * t17;

    return transpose(ret);
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
coordinateSystem(const vec<T, 3> &a)
{
    auto aa = abs(a);
    using V = vec<T, 3>;
    V b{};

    if (aa[0] > aa[1] && aa[0] > aa[2])
        b = V(std::in_place_t{}, -a[2], T{}, a[0]);
    else if (aa[1] > aa[2])
        b = V(std::in_place_t{}, a[1], -a[0], T{});
    else
        b = V(std::in_place_t{}, T{}, a[2], -a[1]);
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

template<size_t I, typename T, size_t N>
HU_FORCE_INLINE inline const math::vec<T, N> &
get(const math::genmat<T, N> &A)
{
    static_assert(I < N);
    return A[I];
}

template<size_t I, typename T, size_t N>
HU_FORCE_INLINE inline math::vec<T, N> &
get(math::genmat<T, N> &A)
{
    static_assert(I < N);
    return A[I];
}

} // namespace math

namespace std {
BEGIN_NO_WARN_MISMATCHED_TAGS
template<typename T, size_t N>
struct tuple_size<math::genmat<T, N>> : public std::integral_constant<size_t, N>
{};
END_NO_WARN_MISMATCHED_TAGS

template<size_t I, typename T, size_t N>
struct tuple_element<I, math::genmat<T, N>>
{
    using type = math::vec<T, N>;
};

} // namespace std

#endif
