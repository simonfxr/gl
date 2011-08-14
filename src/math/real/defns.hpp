#ifndef MATH_REAL_DEFNS_HPP
#define MATH_REAL_DEFNS_HPP

#include "math/real/type.hpp"

#include <limits>

MATH_BEGIN_NAMESPACE

static const real MATH_CONSTANT PI = 3.1415926f;

static const real MATH_CONSTANT POS_INF = std::numeric_limits<real>::infinity();

static const real MATH_CONSTANT NEG_INF = - POS_INF;

real sqrt(real x) PURE_FUNC;

real recip(real x) PURE_FUNC;

real inverse(real x) PURE_FUNC;

real rsqrt(real x) PURE_FUNC;

real sin(real rad) PURE_FUNC;

real cos(real rad) PURE_FUNC;

real tan(real rad) PURE_FUNC;

real asin(real x) PURE_FUNC;

real acos(real x) PURE_FUNC;

real atan(real x) PURE_FUNC;

real atan2(real x, real y) PURE_FUNC;

void sincos(real rad, real& out_sin, real& out_cos) PURE_FUNC;

real cotan(real rad) PURE_FUNC; // 1/tan

real abs(real x) PURE_FUNC;

real length(real x) PURE_FUNC;

real distance(real x, real y) PURE_FUNC;

real squared(real x) PURE_FUNC;

real cubed(real x) PURE_FUNC;

real pow(real x, int32 n) PURE_FUNC;

real pow(real x, real y) PURE_FUNC;

int32 signum(real x) PURE_FUNC;

// bool signbit(real x) PURE_FUNC;

real wrapPi(real x) PURE_FUNC;

real wrap(real x, real period) PURE_FUNC;

real degToRad(real deg) PURE_FUNC;

real radToDeg(real rad) PURE_FUNC;

//real max(real x, real y) PURE_FUNC;
//
//real min(real x, real y) PURE_FUNC;

real saturate(real x) PURE_FUNC;

real clamp(real x, real lo, real hi) PURE_FUNC;

real smoothstep(real lo_edge, real hi_edge, real x) PURE_FUNC;

real lerp(real a, real b, real t) PURE_FUNC;

MATH_END_NAMESPACE

#endif
