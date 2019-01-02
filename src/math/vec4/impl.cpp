#include "math/real.hpp"
#include "math/vec4/defns.hpp"

MATH_BEGIN_NAMESPACE

constexpr const real *
begin(const vec4_t &v)
{
    return v.components;
}

constexpr real *
begin(vec4_t &v)
{
    return v.components;
}

constexpr vec4_t
vec4(real x, real y, real z, real w)
{
    return { x, y, z, w };
}

constexpr vec4_t
vec4(real a)
{
    return vec4(a, a, a, a);
}

constexpr vec4_t
vec4(const vec3_t &a, real w)
{
    return vec4(a[0], a[1], a[2], w);
}

constexpr vec4_t
vec4(const real a[4])
{
    return vec4(a[0], a[1], a[2], a[3]);
}

constexpr void
load(vec4_t::buffer buf, const vec4_t &v)
{
    buf[0] = v[0];
    buf[1] = v[1];
    buf[2] = v[2];
    buf[3] = v[3];
}

constexpr vec4_t
operator-(const vec4_t &a)
{
    return vec4(-a[0], -a[1], -a[2], -a[3]);
}

constexpr vec4_t
operator+(const vec4_t &a, const vec4_t b)
{
    return vec4(a[0] + b[0], a[1] + b[1], a[2] + b[2], a[3] + b[3]);
}

constexpr vec4_t
operator-(const vec4_t &a, const vec4_t b)
{
    return vec4(a[0] - b[0], a[1] - b[1], a[2] - b[2], a[3] - b[3]);
}

constexpr vec4_t operator*(const vec4_t &v, real a)
{
    return v * vec4(a);
}

constexpr vec4_t operator*(real a, const vec4_t &v)
{
    return v * a;
}

constexpr vec4_t operator*(const vec4_t &a, const vec4_t &b)
{
    return vec4(a[0] * b[0], a[1] * b[1], a[2] * b[2], a[3] * b[3]);
}

constexpr vec4_t
operator/(const vec4_t &v, real a)
{
    return v * math::recip(a);
}

constexpr vec4_t &
operator+=(vec4_t &v, const vec4_t &a)
{
    return v = v + a;
}

constexpr vec4_t &
operator-=(vec4_t &v, const vec4_t &a)
{
    return v = v - a;
}

constexpr vec4_t &
operator*=(vec4_t &v, real a)
{
    return v = v * a;
}

constexpr vec4_t &
operator*=(vec4_t &v, const vec4_t &b)
{
    return v = v * b;
}

constexpr vec4_t &
operator/=(vec4_t &v, real a)
{
    return v = v / a;
}

constexpr bool
operator==(const vec4_t &a, const vec4_t &b)
{
    return a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && a[3] == b[3];
}

constexpr bool
operator!=(const vec4_t &a, const vec4_t &b)
{
    return !(a == b);
}

constexpr real
dot(const vec4_t &a, const vec4_t &b)
{
    return sum(a * b);
}

real
length(const vec4_t &a)
{
    return math::sqrt(lengthSq(a));
}

real
inverseLength(const vec4_t &a)
{
    return math::rsqrt(lengthSq(a));
}

constexpr real
lengthSq(const vec4_t &a)
{
    return dot(a, a);
}

vec4_t
normalize(const vec4_t &a)
{
    return a * inverseLength(a);
}

real
distance(const vec4_t &a, const vec4_t &b)
{
    return length(a - b);
}

real
inverseDistance(const vec4_t &a, const vec4_t &b)
{
    return inverseLength(a - b);
}

constexpr real
distanceSq(const vec4_t &a, const vec4_t &b)
{
    return lengthSq(a - b);
}

constexpr vec4_t
min(const vec4_t &a, const vec4_t &b)
{
    return vec4(b[0] < a[0] ? b[0] : a[0],
                b[1] < a[1] ? b[1] : a[1],
                b[2] < a[2] ? b[2] : a[2],
                b[3] < a[3] ? b[3] : a[3]);
}

constexpr vec4_t
max(const vec4_t &a, const vec4_t &b)
{
    return vec4(b[0] > a[0] ? b[0] : a[0],
                b[1] > a[1] ? b[1] : a[1],
                b[2] > a[2] ? b[2] : a[2],
                b[3] > a[3] ? b[3] : a[3]);
}

constexpr real
sum(const vec4_t &a)
{
    return a[0] + a[1] + a[2] + a[3];
}

constexpr bool
equal(const vec4_t &a, const vec4_t &b, real epsi)
{
    return distance(a[0], b[0]) < epsi && distance(a[1], b[1]) < epsi &&
           distance(a[2], b[2]) < epsi && distance(a[3], b[3]) < epsi;
}

MATH_END_NAMESPACE

namespace math {

constexpr MATH_INLINE_SPEC real &vec4_t::operator[](defs::index_t i)
{
    return components[i];
}

constexpr MATH_INLINE_SPEC real vec4_t::operator[](defs::index_t i) const
{
    return components[i];
}

} // namespace math
