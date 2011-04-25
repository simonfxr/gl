#include "math/defs.hpp"

#if defined(MATH_VEC4_INLINE) || !defined(MATH_INLINE)

#include "math/math.hpp"
#include "math/vec4.hpp"

#if MATH_SSE(2, 0)
#include <xmmintrin.h>
#endif

#if MATH_SSE(3, 0)
#include <pmmintrin.h>
#endif

#if MATH_SSE(4, 1)
#include <smmintrin.h>
#endif

MATH_BEGIN_NAMESPACE

vec4_t vec4(float x, float y, float z, float w) {
    vec4_t v; v.x = x; v.y = y; v.z = z; v.w = w; return v;
}

vec4_t vec4(float a) {
#if MATH_SSE(2, 0)
    vec4_t v; v.packed = _mm_set1_ps(a); return v;
#else
    return vec4(a, a, a, a);
#endif
}

vec4_t vec4(const vec3_t& a, float w) {
    return vec4(a.x, a.y, a.z, w);
}

vec4_t vec4(const float a[4]) {
#if MATH_SSE(2, 0)
    vec4_t v; v.packed = _mm_loadu_ps(a); return v;
#else
    return vec4(a[0], a[1], a[2], a[3]);
#endif
}

#if MATH_SSE(2, 0)
vec4_t vec4(__m128 xmm) {
    vec4_t v; v.packed = xmm; return v;
}
#endif

vec4_t operator -(const vec4_t& a) {
#if MATH_SSE(2, 0)
    static const vec4_t negative_zero = vec4(-0.f);
    vec4_t v; v.packed = _mm_xor_ps(negative_zero.packed, a.packed); return v;
#else
    return vec4(0.f) - a;
#endif
}

vec4_t operator +(const vec4_t& a, const vec4_t b) {
#if MATH_SSE(2, 0)
    vec4_t v; v.packed = _mm_add_ps(a.packed, b.packed); return v;
#else
    return vec4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
#endif
}

vec4_t operator -(const vec4_t& a, const vec4_t b) {
#if MATH_SSE(2, 0)
    vec4_t v; v.packed = _mm_sub_ps(a.packed, b.packed); return v;
#else
    return vec4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
#endif  
}

vec4_t operator *(const vec4_t& v, float a) {
    return v * vec4(a);
}

vec4_t operator *(float a, const vec4_t& v) {
    return v * a;
}

vec4_t operator *(const vec4_t& a, const vec4_t& b) {
#if MATH_SSE(2, 0)
    vec4_t v; v.packed = _mm_mul_ps(a.packed, b.packed); return v;
#else
    return vec4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
#endif 
}

vec4_t operator /(const vec4_t& v, float a) {
    return v * math::recip(a);
}

vec4_t& operator +=(vec4_t& v, const vec4_t& a) {
    return v = v + a;
}

vec4_t& operator -=(vec4_t& v, const vec4_t& a) {
    return v = v - a;
}

vec4_t& operator *=(vec4_t& v, float a) {
    return v = v * a;
}

vec4_t& operator *=(vec4_t& v, const vec4_t& b) {
    return v = v * b;
}

vec4_t& operator /=(vec4_t& v, float a) {
    return v = v / a;
}

bool operator ==(const vec4_t& a, const vec4_t& b) {
    return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

bool operator !=(const vec4_t& a, const vec4_t& b) {
    return !(a == b);
}

float dot(const vec4_t& a, const vec4_t& b) {
#if MATH_SSE(4, 1)
    return _mm_cvtss_f32(_mm_dp_ps(a.packed, b.packed, 0xF1));
#else
    return sum(a * b);
#endif  
}

float length(const vec4_t& a) {
    return math::sqrt(lengthSq(a));
}

float inverseLength(const vec4_t& a) {
    return math::rsqrt(lengthSq(a));
}

float lengthSq(const vec4_t& a) {
    return dot(a, a);
}

vec4_t normalize(const vec4_t& a) {
    return a * inverseLength(a);
}

float distance(const vec4_t& a, const vec4_t& b) {
    return length(a - b);
}

float inverseDistance(const vec4_t& a, const vec4_t& b) {
    return inverseLength(a - b);
}

float distanceSq(const vec4_t& a, const vec4_t& b) {
    return lengthSq(a - b);
}

vec4_t min(const vec4_t& a, const vec4_t& b) {
#if MATH_SSE(2, 0)
    vec4_t v; v.packed = _mm_min_ps(a.packed, b.packed); return v;
#else
    return vec4(b.x < a.x ? b.x : a.x,
                b.y < a.y ? b.y : a.y,
                b.z < a.z ? b.z : a.z,
                b.w < a.w ? b.w : a.w);
#endif
}

vec4_t max(const vec4_t& a, const vec4_t& b) {
#if MATH_SSE(2, 0)
    vec4_t v; v.packed = _mm_max_ps(a.packed, b.packed); return v;
#else
    return vec4(b.x > a.x ? b.x : a.x,
                b.y > a.y ? b.y : a.y,
                b.z > a.z ? b.z : a.z,
                b.w > a.w ? b.w : a.w);
#endif
}

float sum(const vec4_t& a) {
#if MATH_SSE(3, 0)
    __m128 b = _mm_hadd_ps(a.packed, a.packed);
    return _mm_cvtss_f32(_mm_hadd_ps(b, b));
#else
    return a.x + a.y + a.z + a.w;
#endif
}

bool equal(const vec4_t& a, const vec4_t& b, float epsi) {
    return distance(a[0], b[0]) < epsi && distance(a[1], b[1]) < epsi &&
        distance(a[2], b[2]) < epsi && distance(a[3], b[3]) < epsi;
}

MATH_END_NAMESPACE

namespace math {

MATH_INLINE_SPEC float& vec4_t::operator[](unsigned long i) {
    return (&x)[i];
//    return components[i];
}

MATH_INLINE_SPEC float vec4_t::operator[](unsigned long i) const {
    return (&x)[i];
//    return components[i];
}

} // namespace math

#endif
