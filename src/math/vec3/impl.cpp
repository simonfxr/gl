#include "math/ivec3.hpp"
#include "math/real.hpp"
#include "math/vec3/defns.hpp"
#include "math/vec4.hpp"

MATH_BEGIN_NAMESPACE

constexpr vec3_t
vec3(real x, real y, real z)
{
    vec3_t v{};
    v[0] = x;
    v[1] = y;
    v[2] = z;
    return v;
}

constexpr vec3_t
vec3(real a)
{
    return vec3(a, a, a);
}

constexpr vec3_t
vec3(const ivec3_t &a)
{
    return vec3(real(a[0]), real(a[1]), real(a[2]));
}

constexpr vec3_t
vec3(const vec4_t &a)
{
    return vec3(a[0], a[1], a[2]);
}

constexpr vec3_t
vec3(const vec3_t::buffer a)
{
    return vec3(a[0], a[1], a[2]);
}

constexpr void
load(vec3_t::buffer b, const vec3_t &v)
{
    for (defs::index i = 0; i < vec3_t::size; ++i)
        b[i] = v[i];
}

constexpr vec3_t
operator-(const vec3_t &a)
{
    return vec3(real(0)) - a;
}

constexpr vec3_t
operator+(const vec3_t &a, const vec3_t b)
{
    return vec3(a[0] + b[0], a[1] + b[1], a[2] + b[2]);
}

constexpr vec3_t
operator-(const vec3_t &a, const vec3_t b)
{
    return vec3(a[0] - b[0], a[1] - b[1], a[2] - b[2]);
}

constexpr vec3_t operator*(const vec3_t &v, real a)
{
    return v * vec3(a);
}

constexpr vec3_t operator*(real a, const vec3_t &v)
{
    return v * a;
}

constexpr vec3_t operator*(const vec3_t &a, const vec3_t &b)
{
    return vec3(a[0] * b[0], a[1] * b[1], a[2] * b[2]);
}

constexpr vec3_t
operator/(const vec3_t &v, real a)
{
    return v * math::recip(a);
}

constexpr vec3_t
operator/(const vec3_t &a, const vec3_t &b)
{
    return a * recip(b);
}

constexpr vec3_t &
operator+=(vec3_t &v, const vec3_t &a)
{
    return v = v + a;
}

constexpr vec3_t &
operator-=(vec3_t &v, const vec3_t &a)
{
    return v = v - a;
}

constexpr vec3_t &
operator*=(vec3_t &v, real a)
{
    return v = v * a;
}

constexpr vec3_t &
operator*=(vec3_t &v, const vec3_t &b)
{
    return v = v * b;
}

constexpr vec3_t &
operator/=(vec3_t &v, real a)
{
    return v = v / a;
}

constexpr vec3_t &
operator/=(vec3_t &v, const vec3_t &b)
{
    return v = v / b;
}

constexpr bool
operator==(const vec3_t &a, const vec3_t &b)
{
    return a[0] == b[0] && a[1] == b[1] && a[2] == b[2];
}

constexpr bool
operator!=(const vec3_t &a, const vec3_t &b)
{
    return !(a == b);
}

constexpr real
dot(const vec3_t &a, const vec3_t &b)
{
    return sum(a * b);
}

constexpr vec3_t
cross(const vec3_t &a, const vec3_t &b)
{
    return vec3(a[1] * b[2] - a[2] * b[1],
                a[2] * b[0] - a[0] * b[2],
                a[0] * b[1] - a[1] * b[0]);
}

real
length(const vec3_t &a)
{
    return math::sqrt(lengthSq(a));
}

real
inverseLength(const vec3_t &a)
{
    return math::rsqrt(lengthSq(a));
}

constexpr real
lengthSq(const vec3_t &a)
{
    return dot(a, a);
}

direction3_t
normalize(const vec3_t &a)
{
    return a * inverseLength(a);
}

real
distance(const vec3_t &a, const vec3_t &b)
{
    return length(a - b);
}

real
inverseDistance(const vec3_t &a, const vec3_t &b)
{
    return inverseLength(a - b);
}

constexpr real
distanceSq(const vec3_t &a, const vec3_t &b)
{
    return lengthSq(a - b);
}

constexpr vec3_t
reflect(const vec3_t &a, const normal3_t &n)
{
    return reflect(a, n, real(1));
}

constexpr vec3_t
reflect(const vec3_t &a, const normal3_t &n, real amp)
{
    return a - n * (real(2) * amp * dot(n, a));
}

constexpr vec3_t
min(const vec3_t &a, const vec3_t &b)
{
    return vec3(b[0] < a[0] ? b[0] : a[0],
                b[1] < a[1] ? b[1] : a[1],
                b[2] < a[2] ? b[2] : a[2]);
}

constexpr vec3_t
max(const vec3_t &a, const vec3_t &b)
{
    return vec3(b[0] > a[0] ? b[0] : a[0],
                b[1] > a[1] ? b[1] : a[1],
                b[2] > a[2] ? b[2] : a[2]);
}

constexpr real
sum(const vec3_t &a)
{
    return a[0] + a[1] + a[2];
}

constexpr vec3_t
recip(const vec3_t &a)
{
    return vec3(recip(a[0]), recip(a[1]), recip(a[2]));
}

constexpr vec3_t
abs(const vec3_t &a)
{
    return vec3(abs(a[0]), abs(a[1]), abs(a[2]));
}

constexpr vec3_t
linearInterpolate(const vec3_t &a, const vec3_t &b, real t)
{
    return a + t * (b - a);
}

direction3_t
directionFromTo(const point3_t &a, const point3_t &b)
{
    return normalize(b - a);
}

real
cos(const vec3_t &a, const vec3_t &b)
{
    return dot(a, b) / (length(a) * length(b));
}

constexpr vec3_t
projectAlong(const vec3_t &a, const vec3_t &x)
{
    return (dot(a, x) / lengthSq(x)) * x;
}

constexpr bool
equal(const vec3_t &a, const vec3_t &b, real epsi)
{
    return distance(a[0], b[0]) < epsi && distance(a[1], b[1]) < epsi &&
           distance(a[2], b[2]) < epsi;
}

MATH_END_NAMESPACE

namespace math {

constexpr MATH_INLINE_SPEC real &vec3_t::operator[](defs::index i)
{
    return components[i];
}

constexpr MATH_INLINE_SPEC real vec3_t::operator[](defs::index i) const
{
    return components[i];
}

} // namespace math
