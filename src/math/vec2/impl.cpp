#include "math/real.hpp"
#include "math/vec2/defns.hpp"

MATH_BEGIN_NAMESPACE

constexpr vec2_t
vec2(real x, real y)
{
    return { x, y };
}

constexpr vec2_t
vec2(real a)
{
    return vec2(a, a);
}

constexpr vec2_t
vec2(const vec3_t &a)
{
    return vec2(a[0], a[1]);
}
constexpr vec2_t
vec2(const vec4_t &a)
{
    return vec2(a[0], a[1]);
}

constexpr vec2_t
vec2(const real a[2])
{
    return vec2(a[0], a[1]);
}

constexpr void
load(vec2_t::buffer buf, const vec2_t &v)
{
    buf[0] = v[0];
    buf[1] = v[1];
}

constexpr vec2_t
operator-(const vec2_t &a)
{
    return vec2(real(0)) - a;
}

constexpr vec2_t
operator+(const vec2_t &a, const vec2_t b)
{
    return vec2(a[0] + b[0], a[1] + b[1]);
}

constexpr vec2_t
operator-(const vec2_t &a, const vec2_t b)
{
    return vec2(a[0] - b[0], a[1] - b[1]);
}

constexpr vec2_t operator*(const vec2_t &v, real a)
{
    return v * vec2(a);
}

constexpr vec2_t operator*(real a, const vec2_t &v)
{
    return v * a;
}

constexpr vec2_t operator*(const vec2_t &a, const vec2_t &b)
{
    return vec2(a[0] * b[0], a[1] * b[1]);
}

constexpr vec2_t
operator/(const vec2_t &v, real a)
{
    return v * math::recip(a);
}

constexpr vec2_t
operator/(const vec2_t &v, const vec2_t &a)
{
    return v * math::recip(a);
}

constexpr vec2_t &
operator+=(vec2_t &v, const vec2_t &a)
{
    return v = v + a;
}

constexpr vec2_t &
operator-=(vec2_t &v, const vec2_t &a)
{
    return v = v - a;
}

constexpr vec2_t &
operator*=(vec2_t &v, real a)
{
    return v = v * a;
}

constexpr vec2_t &
operator*=(vec2_t &v, const vec2_t &b)
{
    return v = v * b;
}

constexpr vec2_t &
operator/=(vec2_t &v, real a)
{
    return v = v / a;
}

constexpr bool
operator==(const vec2_t &a, const vec2_t &b)
{
    return a[0] == b[0] && a[1] == b[1];
}

constexpr bool
operator!=(const vec2_t &a, const vec2_t &b)
{
    return !(a == b);
}

constexpr real
dot(const vec2_t &a, const vec2_t &b)
{
    return sum(a * b);
}

real
length(const vec2_t &a)
{
    return math::sqrt(lengthSq(a));
}

real
inverseLength(const vec2_t &a)
{
    return math::rsqrt(lengthSq(a));
}

constexpr real
lengthSq(const vec2_t &a)
{
    return dot(a, a);
}

direction2_t
normalize(const vec2_t &a)
{
    return a * inverseLength(a);
}

real
distance(const vec2_t &a, const vec2_t &b)
{
    return length(a - b);
}

real
inverseDistance(const vec2_t &a, const vec2_t &b)
{
    return inverseLength(a - b);
}

constexpr real
distanceSq(const vec2_t &a, const vec2_t &b)
{
    return lengthSq(a - b);
}

constexpr vec2_t
reflect(const vec2_t &a, const normal2_t &n)
{
    return reflect(a, n, 1);
}

constexpr vec2_t
reflect(const vec2_t &a, const normal2_t &n, real amp)
{
    return a - n * (2 * amp * dot(n, a));
}

constexpr vec2_t
min(const vec2_t &a, const vec2_t &b)
{
    return vec2(b[0] < a[0] ? b[0] : a[0], b[1] < a[1] ? b[1] : a[1]);
}

constexpr vec2_t
max(const vec2_t &a, const vec2_t &b)
{
    return vec2(b[0] > a[0] ? b[0] : a[0], b[1] > a[1] ? b[1] : a[1]);
}

constexpr real
sum(const vec2_t &a)
{
    return a[0] + a[1];
}

constexpr vec2_t
recip(const vec2_t &a)
{
    return vec2(real(1) / a[0], real(1) / a[1]);
}

constexpr vec2_t
linearInterpolate(const vec2_t &a, const vec2_t &b, real t)
{
    return a + t * (b - a);
}

direction2_t
directionFromTo(const point2_t &a, const point2_t &b)
{
    return normalize(b - a);
}

real
cos(const vec2_t &a, const vec2_t &b)
{
    return dot(a, b) / (length(a) * length(b));
}

constexpr vec2_t
projectAlong(const vec2_t &a, const vec2_t &x)
{
    return (dot(a, x) / lengthSq(x)) * x;
}

constexpr bool
equal(const vec2_t &a, const vec2_t &b, real epsi)
{
    return distance(a[0], b[0]) < epsi && distance(a[1], b[1]) < epsi;
}

MATH_END_NAMESPACE

namespace math {

constexpr MATH_INLINE_SPEC real &vec2_t::operator[](defs::index i)
{
    return components[i];
}

constexpr MATH_INLINE_SPEC real vec2_t::operator[](defs::index i) const
{
    return components[i];
}

} // namespace math
