#include "math/ivec3.hpp"
#include "math/real.hpp"

MATH_BEGIN_NAMESPACE

constexpr ivec3_t
ivec3(defs::int32_t x, defs::int32_t y, defs::int32_t z)
{
    return { x, y, z };
}

constexpr ivec3_t
ivec3(defs::int32_t a)
{
    return ivec3(a, a, a);
}

constexpr ivec3_t
ivec3(const vec3_t &a)
{
    return ivec3(defs::int32_t(a[0]), defs::int32_t(a[1]), defs::int32_t(a[2]));
}

constexpr ivec3_t
ivec3(const defs::int32_t a[3])
{
    return ivec3(a[0], a[1], a[2]);
}

constexpr ivec3_t
operator-(const ivec3_t &a)
{
    return ivec3(defs::int32_t(0)) - a;
}

constexpr ivec3_t
operator+(const ivec3_t &a, const ivec3_t b)
{
    return ivec3(a[0] + b[0], a[1] + b[1], a[2] + b[2]);
}

constexpr ivec3_t
operator-(const ivec3_t &a, const ivec3_t b)
{
    return ivec3(a[0] - b[0], a[1] - b[1], a[2] - b[2]);
}

constexpr ivec3_t operator*(const ivec3_t &v, defs::int32_t a)
{
    return v * ivec3(a);
}

constexpr ivec3_t operator*(defs::int32_t a, const ivec3_t &v)
{
    return v * a;
}

constexpr ivec3_t operator*(const ivec3_t &a, const ivec3_t &b)
{
    return ivec3(a[0] * b[0], a[1] * b[1], a[2] * b[2]);
}

constexpr ivec3_t
operator/(const ivec3_t &v, defs::int32_t a)
{
    return v / ivec3(a);
}

constexpr ivec3_t
operator/(const ivec3_t &a, const ivec3_t &b)
{
    return ivec3(a[0] / b[0], a[1] / b[1], a[2] / b[2]);
}

constexpr ivec3_t &
operator+=(ivec3_t &v, const ivec3_t &a)
{
    return v = v + a;
}

constexpr ivec3_t &
operator-=(ivec3_t &v, const ivec3_t &a)
{
    return v = v - a;
}

constexpr ivec3_t &
operator*=(ivec3_t &v, defs::int32_t a)
{
    return v = v * a;
}

constexpr ivec3_t &
operator*=(ivec3_t &v, const ivec3_t &b)
{
    return v = v * b;
}

constexpr ivec3_t &
operator/=(ivec3_t &v, defs::int32_t a)
{
    return v = v / a;
}

constexpr ivec3_t &
operator/=(ivec3_t &v, const ivec3_t &b)
{
    return v = v / b;
}

constexpr bool
operator==(const ivec3_t &a, const ivec3_t &b)
{
    return a[0] == b[0] && a[1] == b[1] && a[2] == b[2];
}

constexpr bool
operator!=(const ivec3_t &a, const ivec3_t &b)
{
    return !(a == b);
}

constexpr defs::int32_t
dot(const ivec3_t &a, const ivec3_t &b)
{
    return sum(a * b);
}

constexpr ivec3_t
cross(const ivec3_t &a, const ivec3_t &b)
{
    return ivec3(a[1] * b[2] - a[2] * b[1],
                 a[2] * b[0] - a[0] * b[2],
                 a[0] * b[1] - a[1] * b[0]);
}

real
length(const ivec3_t &a)
{
    return math::sqrt(real(lengthSq(a)));
}

real
inverseLength(const ivec3_t &a)
{
    return math::rsqrt(real(lengthSq(a)));
}

constexpr defs::int32_t
lengthSq(const ivec3_t &a)
{
    return dot(a, a);
}

real
distance(const ivec3_t &a, const ivec3_t &b)
{
    return length(a - b);
}

real
inverseDistance(const ivec3_t &a, const ivec3_t &b)
{
    return inverseLength(a - b);
}

constexpr defs::int32_t
distanceSq(const ivec3_t &a, const ivec3_t &b)
{
    return lengthSq(a - b);
}

constexpr ivec3_t
min(const ivec3_t &a, const ivec3_t &b)
{
    return ivec3(b[0] < a[0] ? b[0] : a[0],
                 b[1] < a[1] ? b[1] : a[1],
                 b[2] < a[2] ? b[2] : a[2]);
}

constexpr ivec3_t
max(const ivec3_t &a, const ivec3_t &b)
{
    return ivec3(b[0] > a[0] ? b[0] : a[0],
                 b[1] > a[1] ? b[1] : a[1],
                 b[2] > a[2] ? b[2] : a[2]);
}

constexpr defs::int32_t
sum(const ivec3_t &a)
{
    return a[0] + a[1] + a[2];
}

constexpr bool
equal(const ivec3_t &a, const ivec3_t &b)
{
    return a == b;
}

MATH_END_NAMESPACE

namespace math {

constexpr MATH_INLINE_SPEC defs::int32_t &ivec3_t::operator[](defs::index_t i)
{
    return components[i];
}

constexpr MATH_INLINE_SPEC defs::int32_t ivec3_t::operator[](defs::index_t i) const
{
    return components[i];
}

} // namespace math
