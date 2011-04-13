#if defined(MATH_QUAT_INLINE) || !defined(MATH_INLINE)

#include "math/quat/defns.hpp"
#include "math/vec4.hpp"
#include "math/math.hpp"

MATH_BEGIN_NAMESPACE

quat_t quat() {
    return quat(vec4(1.f, 0.f, 0.f, 0.f));
}

quat_t quat(const vec4_t& coefficients) {
    quat_t q; q.coeff = coefficients; return q;
}

float length(const quat_t& q) {
    return length(q.coeff);
}

float lengthSq(const quat_t& q) {
    return lengthSq(q.coeff);
}

float inverseLength(const quat_t& q) {
    return inverseLength(q.coeff);
}

quat1_t normalize(const quat_t& q) {
    return quat(normalize(q.coeff));
}

quat_t inverse(const quat_t& q) {
    quat_t conj = conjugate(q);
    conj.coeff /= lengthSq(q.coeff);
    return conj;
}

quat_t conjugate(const quat_t& q) {
    quat_t conj = q;
    conj.coeff = - conj.coeff;
    conj.a = - conj.a;
    return conj;
}

quat_t operator -(const quat_t& p) {
    return quat(-p.coeff);
}

quat_t operator *(const quat_t& p, const quat_t& q) {
    quat_t r;
    r.a = p.a * q.a - p.b * q.b - p.c * q.c - p.d * q.d;
    r.b = p.a * q.b + p.b * q.a + p.c * q.d - p.d * q.c;
    r.c = p.a * q.c - p.b * q.d + p.c * q.a + p.d * q.b;
    r.d = p.a * q.d + p.b * q.c - p.c * q.b + p.d * q.a;
    return r;
}

quat_t& operator *=(quat_t& p, const quat_t& q) {
    return p = p * q;
}

// 3D Math Primer for Graphics and Game Development (3dmp) (great book btw.)
quat_t1 pow(const quat1_t& q, float exp) {

    if (abs(q.a) < 0.9999f) {

        float alpha = acos(q.a);
        float newAlpha = alpha * exp;

        float s, c;
        sincos(newAlpha, s, c);
        
        quat r;
        r.a = 0.f;

        float mult = s / sin(alpha);
        r.coeff *= mult;
        r.a = c;

        return r;
    } else {
        return quat();
    }
}

// also 3dmp
quat_t1 slerp(const quat1_t& p, const quat1_t& q) {

    float w0 = p.a, x0 = p.b, y0 = p.c, z0 = p.d;
    float w1 = q.a, x1 = q.b, y1 = q.c, z1 = q.d;

// The output quaternion will be computed here
    float w,x,y,z;
// Compute the "cosine of the angle" between the
// quaternions, using the dot product
    float cosOmega = w0*w1 + x0*x1 + y0*y1 + z0*z1;
// If negative dot, negate one of the input
// quaternions to take the shorter 4D "arc"
    if (cosOmega < 0.0f) {
        w1 = -w1;
        x1 = -x1;
        y1 = -y1;
        z1 = -z1;
        cosOmega = -cosOmega;
    }
// Check if they are very close together to protect
// against divide-by-zero
    float k0, k1;
    if (cosOmega > 0.9999f) {
// Very close - just use linear interpolation
        k0 = 1.0f-t;
        k1 = t;
    } else {
// Compute the sin of the angle using the
// trig identity sin^2(omega) + cos^2(omega) = 1
        float sinOmega = sqrt(1.0f - cosOmega*cosOmega);
// Compute the angle from its sin and cosine
        float omega = atan2(sinOmega, cosOmega);
// Compute inverse of denominator, so we only have
// to divide once
        float oneOverSinOmega = 1.0f / sinOmega;
// Compute interpolation parameters

        k0 = sin((1.0f - t) * omega) * oneOverSinOmega;
        k1 = sin(t * omega) * oneOverSinOmega;
    }
// Interpolate
    w = w0*k0 + w1*k1;
    x = x0*k0 + x1*k1;
    y = y0*k0 + y1*k1;
    z = z0*k0 + z1*k1;

    return quat(vec4(w, x, y, z));
}

MATH_END_NAMESPACE

#endif

